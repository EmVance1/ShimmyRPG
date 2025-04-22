#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"
#include "pch.h"
#include "iso_math.h"
#include "texmap.h"
#include "repr.h"
#include "sorting.h"
#include "uuid.h"
#include "sf_json.h"

namespace nl = nlohmann;


struct Level {
    sf::Sprite background;
    std::unordered_map<std::string, Entity> entities;
    std::vector<Trigger> triggers;
    sf::RenderTexture pathmap;
    sf::Vector2f topleft;
    float scale = 1.f;

    Level(const sf::Texture& tex) : background(tex) {}
};

enum class Layer {
    Trigger = 1 << 0,
    Entity  = 1 << 1,
    PathMap = 1 << 2,
};


Level load_level_file(const std::string& filename) {
    auto area_file = std::ifstream(filename);
    auto area = nl::json::parse(area_file);

    TextureMap::load_texture("background", area["world"]["background"]);
    auto level = Level(TextureMap::get_texture("background"));
    auto _ = level.pathmap.resize({1920, 1080});
    level.topleft = json_to_vector2f(area["world"]["topleft"]);
    level.scale = area["world"]["scale"];
    for (const auto& e : area["entities"]) {
        const auto uuid = Uuid::generate_v4();
        level.entities[uuid] = Entity::from_json(e);
        level.entities[uuid].uuid = uuid;
    }
    for (const auto& t : area["triggers"]) {
        level.triggers.push_back(Trigger::from_json(t));
    }
    return level;
}

void save_level_file(const std::string& filename, const Level& level) {
    auto json = nm::json();

    json["world"]["topleft"] = vector2f_to_json(level.topleft);
    json["world"]["scale"] = level.scale;

    json["entities"] = nm::json::array();
    for (const auto& [_, e] : level.entities) {
        json["entities"].push_back(e.into_json());
    }
    json["triggers"] = nm::json::array();
    for (const auto& t : level.triggers) {
        json["triggers"].push_back(t.into_json());
    }

    auto file = std::ofstream("res/world/" + filename + ".json");
    file << json;
}



void EntityMenu(Entity& e, bool is_active, const sf::Vector2f& topleft, float scale) {
    ImGui::Begin("Entity Editor", &is_active, ImGuiWindowFlags_MenuBar);

    ImGui::InputFloat2("position", (float*)&e.position.pos);

    Position last = e.position;
    if (ImGui::RadioButton("grid",      (int*)&e.position.mode, (int)Position::Mode::Grid) ||
        ImGui::RadioButton("world",     (int*)&e.position.mode, (int)Position::Mode::World) ||
        ImGui::RadioButton("world_iso", (int*)&e.position.mode, (int)Position::Mode::WorldIso)) {
        e.position = map_spaces(last, e.position.mode, topleft, scale);
    }

    ImGui::BeginListBox("tags");
    for (auto& t : e.tags) {
        ImGui::Text("%s", t.c_str());
    }
    ImGui::EndListBox();
    static std::string input_tag = "";
    ImGui::InputText("new tag", &input_tag);
    if (ImGui::Button("+")) {
        e.tags.push_back(input_tag);
        input_tag = "";
    }

    ImGui::End();
}

void TriggerMenu(Trigger& t, bool is_active, bool& delete_active) {
    ImGui::Begin("Trigger Editor", &is_active, ImGuiWindowFlags_MenuBar);

    ImGui::InputText("name", &t.id);
    ImGui::InputFloat2("position", (float*)&t.bounds.position);
    ImGui::InputFloat2("size",     (float*)&t.bounds.size);
    ImGui::InputText("active_if", &t.active_if);

    ImGui::End();
}


int main() {
    auto ctx = sf::ContextSettings();
    ctx.antiAliasingLevel = 8;
    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Shimmy Editor", sf::Style::Default, sf::State::Windowed, ctx);
    window.setPosition({0, 0});
    auto clock = sf::Clock();

    auto _ = ImGui::SFML::Init(window);
    ImGui::GetIO().Fonts->Clear();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("res/fonts/calibri.ttf", 20.f);
    _ = ImGui::SFML::UpdateFontTexture();

    // auto active_layer = Layer::Trigger;
    auto visible_layers = (int)Layer::Trigger|(int)Layer::Entity|(int)Layer::PathMap;

    auto texture_file = std::ifstream("res/world/ademmar/textures.json");
    auto textures = nl::json::parse(texture_file);
    for (const auto& [k, v] : textures.items()) {
        TextureMap::load_texture(k, v);
    }

    auto level = load_level_file("res/world/ademmar/fountainroad.json");

    const auto grid_to_cart = grid_to_cartesian(level.scale);
    const auto cart_to_iso  = cartesian_to_isometric(level.topleft);
    const auto grid_to_iso  = grid_to_isometric(level.topleft, level.scale);
    const auto iso_to_cart  = isometric_to_cartesian(level.topleft);
    const auto iso_to_grid  = isometric_to_grid(level.topleft, level.scale);

    auto bg_map    = sf::Sprite(TextureMap::get_texture("pathmap"));

    Trigger* selected_trigger = nullptr;
    Entity* selected_entity = nullptr;
    bool mouse_held = false;
    sf::Vector2f clickdiff;

    bool show_export = false;
    std::string filename = "";

    while (auto event = window.waitEvent()) {
        const auto deltatime = clock.restart();

        ImGui::SFML::ProcessEvent(window, *event);

        if (event->is<sf::Event::Closed>()) {
            window.close();
            ImGui::SFML::Shutdown();
            return 0;
        } else if (auto mbp = event->getIf<sf::Event::MouseButtonPressed>()) {
            mouse_held = true;
            clickdiff = sf::Vector2f(0, 0);
            const auto cart = iso_to_cart.transformPoint(sf::Vector2f(mbp->position));
            for (auto& t : level.triggers) {
                t.outline.setOutlineThickness(1.f);
                if (t.bounds.contains(cart)) {
                    if (selected_trigger) {
                        selected_trigger->outline.setOutlineThickness(1.f);
                    }
                    if (selected_entity) {
                        selected_entity->outline.setOutlineThickness(1.f);
                        selected_entity = nullptr;
                    }
                    selected_trigger = &t;
                    clickdiff = cart - t.bounds.position;
                    t.outline.setOutlineThickness(2.f);
                }
            }
            for (auto& [_, e] : level.entities) {
                e.outline.setOutlineThickness(1.f);
                if (e.outline.getGlobalBounds().contains(sf::Vector2f(mbp->position))) {
                    if (selected_trigger) {
                        selected_trigger->outline.setOutlineThickness(1.f);
                        selected_trigger = nullptr;
                    }
                    if (selected_entity) {
                        selected_entity->outline.setOutlineThickness(1.f);
                    }
                    selected_entity = &e;
                    clickdiff = sf::Vector2f(mbp->position) - e.outline.getPosition();
                    e.outline.setOutlineThickness(2.f);
                }
            }
        } else if (auto mmv = event->getIf<sf::Event::MouseMoved>()) {
            if (mouse_held) {
                const auto cart = iso_to_cart.transformPoint(sf::Vector2f(mmv->position));
                if (selected_trigger) {
                    selected_trigger->bounds.position = cart - clickdiff;
                } else if (selected_entity) {
                    switch (selected_entity->position.mode) {
                    case Position::Mode::Grid: {
                        const auto grid = iso_to_grid.transformPoint(sf::Vector2f(mmv->position) - clickdiff);
                        selected_entity->position.pos = grid;
                        break; }
                    case Position::Mode::World:
                        selected_entity->position.pos = cart - clickdiff;
                        break;
                    case Position::Mode::WorldIso:
                        selected_entity->position.pos = sf::Vector2f(mmv->position) - clickdiff;
                        break;
                    }
                }
            }
        } else if (auto mbr = event->getIf<sf::Event::MouseButtonReleased>()) {
            mouse_held = false;
        } else if (auto kyp = event->getIf<sf::Event::KeyPressed>()) {
            if (kyp->code == sf::Keyboard::Key::E && kyp->control) {
                show_export = true;
            }
        }

        ImGui::SFML::Update(window, deltatime);

        bool delete_active_trigger = false;
        if (selected_entity) {
            EntityMenu(*selected_entity, true, level.topleft, level.scale);
        } else if (selected_trigger) {
            TriggerMenu(*selected_trigger, true, delete_active_trigger);
        }

        if (show_export) {
            ImGui::Begin("Export", &show_export);
            ImGui::InputText("filename", &filename);
            if (ImGui::Button("save")) {
                show_export = false;
                save_level_file(filename, level);
            }
            ImGui::End();
        }

        window.clear();
        window.draw(level.background);

        auto entities = sprites_topo_sort(level.entities);

        if (visible_layers & (int)Layer::PathMap) {
            level.pathmap.draw(bg_map, grid_to_cart);
            level.pathmap.display();
            auto pm = sf::Sprite(level.pathmap.getTexture());
            pm.setColor(sf::Color(255, 255, 255, 100));
            window.draw(pm, cart_to_iso);
        }
        if (visible_layers & (int)Layer::Trigger) {
            for (auto& trigger : level.triggers) {
                trigger.outline.setPosition(trigger.bounds.position);
                trigger.outline.setSize(trigger.bounds.size);
                window.draw(trigger.outline, cart_to_iso);
            }
        }
        if (visible_layers & (int)Layer::Entity) {
            for (auto& entity : entities) {
                switch (entity->position.mode) {
                case Position::Mode::Grid:
                    entity->sprite.setPosition(grid_to_iso.transformPoint(entity->position.pos));
                    entity->outline.setPosition(grid_to_iso.transformPoint(entity->position.pos));
                    break;
                case Position::Mode::World:
                    entity->sprite.setPosition(cart_to_iso.transformPoint(entity->position.pos));
                    entity->outline.setPosition(cart_to_iso.transformPoint(entity->position.pos));
                    break;
                case Position::Mode::WorldIso:
                    entity->sprite.setPosition(entity->position.pos);
                    entity->outline.setPosition(entity->position.pos);
                    break;
                }
                entity->collider.setPosition(iso_to_cart.transformPoint(entity->sprite.getPosition()));
                window.draw(entity->collider, cart_to_iso);
                window.draw(entity->sprite);
                window.draw(entity->outline);
            }
        }

        ImGui::SFML::Render(window);

        window.display();
    }
}


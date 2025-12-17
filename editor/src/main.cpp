#include "pch.h"
#include "util/iso_math.h"
#include "util/files.h"
#include "navedit/editor.h"


struct Scene {
    std::vector<sf::Texture> bg_textures;
    std::vector<sf::Sprite> background;
    sf::FloatRect bounds;
    sf::Vector2f origin;

    std::unordered_map<std::string, Entity> entities;
    std::vector<Trigger> triggers;
};

enum class Layer {
    Entity  = 1 << 0,
    Trigger = 1 << 1,
    NavMesh = 1 << 2,
};


Scene load_scene(const std::filesystem::path& dir) {
    auto result = Scene{};
    auto mintl = sf::Vector2f(0, 0);
    auto maxbr = sf::Vector2f(0, 0);

    for (const auto& f : std::filesystem::directory_iterator(dir)) {
        if (f.path().extension() != ".png") continue;
        result.bg_textures.emplace_back(f.path());
    }
    size_t i = 0;
    for (const auto& f : std::filesystem::directory_iterator(dir)) {
        if (f.path().extension() != ".png") continue;
        const auto stem = f.path().stem().string();
        const auto split = stem.find('_');
        const auto x_idx = std::atoi(stem.substr(0, split).c_str());
        const auto y_idx = std::atoi(stem.substr(split+1).c_str());
        const auto& tex = result.bg_textures[i];
        const auto size = tex.getSize();
        auto& sprite = result.background.emplace_back(tex);
        const auto tl = sf::Vector2f(x_idx * size.x, y_idx * size.y);
        const auto br = tl + (sf::Vector2f)size;
        mintl.x = std::min(tl.x, mintl.x);
        mintl.y = std::min(tl.y, mintl.y);
        maxbr.x = std::max(br.x, maxbr.x);
        maxbr.y = std::max(br.y, maxbr.y);
        sprite.setPosition(tl);
        i++;
    }

    result.bounds = sf::FloatRect(mintl, maxbr - mintl);
    result.origin = result.bounds.position + result.bounds.size * 0.5f;
    return result;
}


struct ScaleMarker {
    sf::Vector2f begin;
    float size = 100;
    int meters = 1;

    bool left_held  = false;
    bool right_held = false;
    bool total_held = false;
    float drag_offset = 0;

    // units (bg pixels) per meter
    float get_scale() const {
        return size / meters;
    }

    void render(sf::RenderTarget& target) const {
        const auto boxh = 4.f;
        const auto hanh = 5.f;
        auto shape = sf::RectangleShape();
        shape.setPosition({ begin.x, begin.y - boxh * 0.5f });
        shape.setSize({ size, boxh });
        shape.setFillColor(sf::Color::Cyan);
        target.draw(shape);
        auto handle = sf::CircleShape();
        handle.setPosition(begin);
        handle.setOrigin({ hanh, hanh });
        handle.setRadius(hanh);
        handle.setFillColor(sf::Color(0, 150, 150));
        target.draw(handle);
        handle.move({ size, 0 });
        target.draw(handle);
    }
};


int main() {
    auto ctx = sf::ContextSettings();
    ctx.antiAliasingLevel = 4;
    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Shimmy Editor", sf::Style::Default, sf::State::Windowed, ctx);
    window.setPosition({0, 0});
    auto clock = sf::Clock();
    auto view = sf::View(sf::FloatRect{ { 0, 0 }, { 0, 0 } });

    auto _ = ImGui::SFML::Init(window);
    ImGui::GetIO().Fonts->Clear();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("res/calibri.ttf", 20.f);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    _ = ImGui::SFML::UpdateFontTexture();

    auto visible_layers = (int)Layer::Entity|(int)Layer::Trigger|(int)Layer::NavMesh;
    auto drag_held = false;

    auto working_dir = std::filesystem::path();
    if (auto dir = select_folder(window.getNativeHandle())) {
        working_dir = *dir;
    } else {
        return 1;
    }

    auto scene = load_scene(working_dir);
    view.setSize(sf::Vector2f(1920, 1080) * 2.f);
    view.setCenter(scene.origin);

    auto scale_marker = ScaleMarker();
    scale_marker.begin = scene.origin;

    auto nav_editor = NavmeshEditor();
    nav_editor.load_file(working_dir / "shapes.txt");
    scale_marker.size = nav_editor.get_world_scale();

    auto into_iso  = cart_to_iso(scene.origin, 1.f);
    auto into_cart = iso_to_cart(scene.origin, 1.f);

    nav_editor.set_world_transform(into_cart);

    auto origin_marker = sf::ConvexShape(8);
    origin_marker.setFillColor(sf::Color(255, 150, 0));
    origin_marker.setPosition(scene.origin);
    origin_marker.setPoint(0, {   0, -25 });
    origin_marker.setPoint(2, {  25,   0 });
    origin_marker.setPoint(4, {   0,  25 });
    origin_marker.setPoint(6, { -25,   0 });
    origin_marker.setPoint(1, {   3,  -3 });
    origin_marker.setPoint(3, {   3,   3 });
    origin_marker.setPoint(5, {  -3,   3 });
    origin_marker.setPoint(7, {  -3,  -3 });

    auto font = sf::Font("res/calibri.ttf");
    auto scale_bg = sf::RectangleShape({ 100, 30 });
    scale_bg.setPosition({ 15, 15 });
    scale_bg.setFillColor(sf::Color::Black);
    auto scale_display = sf::Text(font, "", 25);
    scale_display.setString("scale: " + std::to_string((int)scale_marker.get_scale()) + "p/m");
    scale_display.setPosition({ 20, 20 });

    while (auto event = window.waitEvent()) {
        const auto deltatime = clock.restart();

        ImGui::SFML::ProcessEvent(window, *event);

        nav_editor.handle_event(*event, window);

        if (event->is<sf::Event::Closed>()) {
            window.close();
            ImGui::SFML::Shutdown();
            return 0;

        } else if (const auto kyp = event->getIf<sf::Event::KeyPressed>()) {
            if (kyp->control) {
                if (kyp->code == sf::Keyboard::Key::S) {
                    nav_editor.save_file(working_dir / "shapes.txt");
                } else if (kyp->code == sf::Keyboard::Key::E) {
                    nav_editor.export_file(working_dir / "scene.nav");
                }
            }
        } else if (const auto mbp = event->getIf<sf::Event::MouseButtonPressed>()) {
            const auto world_pos = window.mapPixelToCoords(mbp->position, view);
            if (mbp->button == sf::Mouse::Button::Left) {
                const auto end = scale_marker.begin + sf::Vector2f(scale_marker.size, 0);
                if ((scale_marker.begin - world_pos).lengthSquared() < 40.f) {
                    scale_marker.left_held = true;
                } else if ((end - world_pos).lengthSquared() < 40.f) {
                    scale_marker.right_held = true;
                } else {
                    const auto tl = scale_marker.begin - sf::Vector2f(5, 5);
                    const auto br = scale_marker.begin + sf::Vector2f(scale_marker.size + 5, 5);
                    const auto box = sf::FloatRect(tl, br - tl);
                    if (box.contains(world_pos)) {
                        scale_marker.total_held = true;
                        scale_marker.drag_offset = world_pos.x - scale_marker.begin.x;
                    }
                }
            } else if (mbp->button == sf::Mouse::Button::Middle) {
                drag_held = true;
            }

        } else if (const auto mmv = event->getIf<sf::Event::MouseMoved>()) {
            const auto world_pos = window.mapPixelToCoords(mmv->position, view);
            if (drag_held) {
                view.setCenter((sf::Vector2f)mmv->position);
            } else if (scale_marker.total_held) {
                scale_marker.begin = sf::Vector2f(world_pos.x - scale_marker.drag_offset, world_pos.y);
            } else if (scale_marker.left_held) {
                scale_marker.size -= world_pos.x - scale_marker.begin.x;
                scale_marker.begin.x = world_pos.x;
                scale_display.setString("scale: " + std::to_string((int)scale_marker.get_scale()) + "p/m");
            } else if (scale_marker.right_held) {
                scale_marker.size = world_pos.x - scale_marker.begin.x;
                scale_display.setString("scale: " + std::to_string((int)scale_marker.get_scale()) + "p/m");
            }
        } else if (event->is<sf::Event::MouseButtonReleased>()) {
            if (scale_marker.left_held || scale_marker.right_held) {
                scale_marker.left_held  = false;
                scale_marker.right_held = false;
                if (scale_marker.size < 0) {
                    scale_marker.begin.x += scale_marker.size;
                    scale_marker.size *= -1;
                }
                nav_editor.set_world_scale(scale_marker.get_scale());
                scale_display.setString("scale: " + std::to_string((int)scale_marker.get_scale()) + "p/m");
            }
            scale_marker.total_held = false;
            drag_held = false;
        } else if (const auto scrl = event->getIf<sf::Event::MouseWheelScrolled>()) {
            const auto begin = window.mapPixelToCoords(scrl->position, view);
            view.zoom((scrl->delta > 0) ? 0.98f : 1.02f);
            const auto end = window.mapPixelToCoords(scrl->position, view);
            view.move(begin - end);
        }

        ImGui::SFML::Update(window, deltatime);

        // ImGuiID dockspace_id = ImGui::GetID("dockspace");
        // ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        window.clear();

        window.setView(view);

        for (const auto& sp : scene.background) {
            window.draw(sp);
        }

        nav_editor.render(window, into_iso);
        scale_marker.render(window);
        window.draw(origin_marker);

        window.setView(window.getDefaultView());
        window.draw(scale_display);
        window.setView(view);

        ImGui::SFML::Render(window);

        window.display();
    }
}


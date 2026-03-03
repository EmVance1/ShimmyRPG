#include "pch.h"
#include "simulate.h"
#include "cinematic.h"
#include "util/env.h"
#include "core/fs.h"
#include "core/split.h"
#include "data/flags.h"
#include "world/game.h"
#include "world/scene.h"


namespace shmy::sim {

void Cinematic::init(Game* game) {
    p_game = game;

    root = ::gui::Panel::create(
        ::gui::Position{ ::gui::lo::xcenter(0.f), ::gui::lo::bottom(-150.f) },
        ::gui::Sizing{ ::gui::lo::percent(50.f), ::gui::lo::absolute(220.f) },
        game->gui.get_style());
    game->gui.add_widget("dialogue", root);
    root->set_enabled(false);
    root->set_visible(false);
    root->set_background_color(sf::Color::Transparent);

    auto speaker = ::gui::Text::create(
        ::gui::Position{ ::gui::lo::right(0.f), ::gui::lo::top(-50.f) },
        ::gui::Sizing({ 300.f, 50.f }),
        game->gui.get_style(), "");
    speaker->set_background_texture(sf::IntRect({250, 0}, {50, 49}));
    speaker->set_border({2, 2});
    speaker->set_sorting_layer(-1);
    speaker->set_text_position(::gui::lo::center({0, 0}));
    root->add_widget("speaker", speaker);


    auto text = ::gui::VerticalList::create(
        ::gui::Position{ ::gui::lo::left(0.f), ::gui::lo::top(0.f) },
        ::gui::Sizing{ ::gui::lo::percent(100.f), ::gui::lo::fitcontent() },
        game->gui.get_style());
    root->add_widget("text", text);
    text->set_background_color(sf::Color::Transparent);

    auto lines = ::gui::Button::create(
        ::gui::Position{ ::gui::lo::left(0.f), ::gui::lo::top(0.f) },
        ::gui::Sizing{ ::gui::lo::percent(100.f), ::gui::lo::fitcontent() },
        game->gui.get_style(), "");
    lines->set_text_padding(10.f);
    lines->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
    lines->set_border({2, 2});
    lines->set_callback([&](){ signal_action(ContinueEvent{}); });
    text->add_widget("lines", lines);

    auto choice = ::gui::VerticalList::create(
        ::gui::Position{ ::gui::lo::left(0.f), ::gui::lo::top(90.f) },
        ::gui::Sizing{ ::gui::lo::percent(100.f), ::gui::lo::absolute(0.f) },
        game->gui.get_style());
    choice->set_background_texture(sf::IntRect({250, 2}, {50, 48}));
    choice->set_border({2, 2});
    choice->set_enabled(false);
    choice->set_visible(false);
    text->add_widget("choices", choice);
}

void Cinematic::show() {
    root->set_enabled(true);
    root->set_visible(true);
}

void Cinematic::hide() {
    root->set_enabled(false);
    root->set_visible(false);
}


void Cinematic::signal_action(const Event& event) {
    auto khook = shmy::data::Flags::key_hook;
    auto vhook = shmy::data::Flags::value_hook;
    auto ehook = Game::entity_hook;

    std::visit([&](auto&& v) {
        using T = std::decay_t<decltype(v)>;

        auto& scene = p_game->active_scene();

        if constexpr (std::is_same_v<T, BeginSpeech>) {
            const auto toks = shmy::core::split(v.modpath, '.');
            const auto src = shmy::core::read_to_string(shmy::env::pkg_full() / (toks[0] + ".qsi")).unwrap();
            shmy::data::Flags::begin_temps();
            quosiError errors;
            quosiSymbolCtx ctx = { .data_lkp=khook, .speaker_lkp=ehook };
            m_current_file = quosi_file_compile_from_src(src.c_str(), &errors, ctx, quosi_malloc_allocator());
            if (errors.list != NULL) {
                for (size_t i = 0; i < quosi_error_list_len(&errors); i++) {
                    printf("quosi compile error (%u:%u): %s\n", errors.list[i].span.row, errors.list[i].span.col, quosi_error_to_string(errors.list[i]));
                }
                quosi_error_list_free(&errors);
                exit(1);
            }
            shmy::data::Flags::end_temps();
            // quosi_file_prettyprint(m_current_file, toks[1].c_str(), stdout);
            quosi_vm_init(&m_interpreter, m_current_file, toks[1].c_str());
            m_once_prefix = "once_quosi_" + v.modpath + "_";
            show();
        } else if constexpr (std::is_same_v<T, SelectEvent>) {
            quosi_vm_push_value(&m_interpreter, v.prop.idx);
        }

repeat:
        const auto once_id = m_once_prefix + std::to_string(m_interpreter.PC);
        shmy::data::Flags::Allow() = !shmy::data::Flags::check_once(once_id);
        const auto upcall = quosi_vm_exec(&m_interpreter, vhook);
        shmy::data::Flags::set_once(once_id);
        switch (upcall) {
        case QUOSI_UPCALL_EVENT:
            scene.lua_vm.load_anon(std::string("DispatchEvent('") + quosi_vm_line(&m_interpreter) + "', {})");
            goto repeat;
        case QUOSI_UPCALL_LINE: {
            auto cont = root->get_widget<::gui::VerticalList>("text");
            auto choice_gui = cont->get_widget("choices");
            choice_gui->set_enabled(false);
            choice_gui->set_visible(false);
            auto speaker_gui = root->get_widget<::gui::TextWidget>("speaker");
            if (quosi_vm_id(&m_interpreter) == UINT32_MAX) {
                speaker_gui->set_label("Narrator");
            } else {
                speaker_gui->set_label(p_game->entity(quosi_vm_id(&m_interpreter)).name());
            }
            auto line_gui = cont->get_widget<::gui::TextWidget>("lines");
            line_gui->set_enabled(true);
            line_gui->set_visible(true);
            line_gui->set_label(quosi_vm_line(&m_interpreter));
            cont->refresh();
            break; }
        case QUOSI_UPCALL_PICK: {
            auto cont = root->get_widget<::gui::VerticalList>("text");
            cont->get_widget<::gui::TextWidget>("lines")->set_enabled(false);
            auto choice_gui = cont->get_widget<::gui::VerticalList>("choices");
            choice_gui->clear();
            choice_gui->set_enabled(true);
            choice_gui->set_visible(true);
            for (uint32_t i = 0; i < quosi_vm_nq(&m_interpreter); i++) {
                const auto prop = quosi_vm_dequeue_text(&m_interpreter);
                auto b = ::gui::Button::create(
                    ::gui::Position({0, 0}),
                    ::gui::Sizing{ ::gui::lo::percent(100), ::gui::lo::absolute(30) },
                    choice_gui->get_style(), std::to_string(i+1) + ". " + prop.str);
                b->set_callback([&, prop](){ signal_action(SelectEvent{ prop }); });
                b->set_text_padding(5.f);
                b->set_background_texture(sf::IntRect{ {400, 0}, {50, 50} });
                b->set_style_variant(1);
                b->set_border({2, 2});
                choice_gui->add_widget(b);
            }
            cont->refresh();
            break; }
        case QUOSI_UPCALL_EXIT: {
            free(m_current_file);
            auto choice_gui = root->get_widget<::gui::VerticalList>("text")->get_widget("choices");
            choice_gui->set_enabled(false);
            choice_gui->set_visible(false);
            if (!manual_exit) {
                p_game->set_mode(Game::Mode::Simulation);
            }
            root->set_enabled(false);
            root->set_visible(false);
            break; }
        }
    }, event);
}

void Cinematic::handle_input(const sf::Event& event) {
    (void)event;
}

void Cinematic::handle_event(const shmy::Event& event) {
    (void)event;
}

void Cinematic::update() {}

}


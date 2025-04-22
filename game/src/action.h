#pragma once
#include <variant>
#include <string>


struct MoveToAction   {};
struct SpeakAction    { std::string target; std::string speech; };
struct AttackAction   {};
struct OpenDoorAction {};
struct OpenInvAction  {};
struct LockpickAction {};
struct PickUpAction   {};
class ContextAction {
public:
    using ContextAction_T = std::variant<MoveToAction, SpeakAction, AttackAction, OpenDoorAction, OpenInvAction, LockpickAction, PickUpAction>;

private:
    ContextAction_T m_impl;

public:
    ContextAction(const ContextAction_T& ctx) : m_impl(ctx) {}

    const ContextAction_T& get_inner() const { return m_impl; }

    const char* to_string() const {
        switch (m_impl.index()) {
        case 0: return "Move To";
        case 1: return "Speak";
        case 2: return "Attack";
        case 3: return "Open";
        case 4: return "Open";
        case 5: return "Pick Lock";
        case 6: return "Pick Up";
        }
        return "";
    }
};


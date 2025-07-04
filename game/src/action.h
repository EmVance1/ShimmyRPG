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

    enum class ActionID {
        MoveTo,
        Speak,
        Attack,
        OpenDoor,
        OpenInv,
        Lockpick,
        PickUp,
    };

private:
    ContextAction_T m_impl;

public:
    ContextAction(const ContextAction_T& ctx) : m_impl(ctx) {}

    const ContextAction_T& get_inner() const { return m_impl; }

    ActionID index() const {
        switch (m_impl.index()) {
        case 0: return ActionID::MoveTo;
        case 1: return ActionID::Speak;
        case 2: return ActionID::Attack;
        case 3: return ActionID::OpenDoor;
        case 4: return ActionID::OpenInv;
        case 5: return ActionID::Lockpick;
        case 6: return ActionID::PickUp;
        }
        return (ActionID)0;
    }

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


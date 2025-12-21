#pragma once
#include <variant>
#include <string>


struct MoveToAction   { std::string target; };
struct SpeakAction    { std::string target; std::string speech; };
struct ExamineAction  { std::string target; std::string speech; };
struct AttackAction   {};
struct OpenDoorAction {};
struct OpenInvAction  {};
struct LockpickAction {};
struct PickUpAction   {};
class ContextAction {
public:
    using ContextAction_T = std::variant<
        MoveToAction,
        SpeakAction,
        ExamineAction,
        AttackAction,
        OpenDoorAction,
        OpenInvAction,
        LockpickAction,
        PickUpAction
    >;

    enum class ActionID {
        MoveTo,
        Speak,
        Examine,
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
        case 2: return ActionID::Examine;
        case 3: return ActionID::Attack;
        case 4: return ActionID::OpenDoor;
        case 5: return ActionID::OpenInv;
        case 6: return ActionID::Lockpick;
        case 7: return ActionID::PickUp;
        }
        return (ActionID)0;
    }

    const char* to_string() const {
        switch (m_impl.index()) {
        case 0: return "Move To";
        case 1: return "Speak";
        case 2: return "Examine";
        case 3: return "Attack";
        case 4: return "Open";
        case 5: return "Open";
        case 6: return "Pick Lock";
        case 7: return "Pick Up";
        }
        return "";
    }

    template<typename T>
    T& get() { return std::get<T>(m_impl); }
    template<typename T>
    const T& get() const { return std::get<T>(m_impl); }
};

bool operator==(const ContextAction& lhs, const ContextAction& rhs);

namespace std { template<> struct hash<ContextAction> {
    size_t operator()(const ContextAction& act) const noexcept { return std::hash<size_t>()((size_t)act.get_inner().index()); }
}; }


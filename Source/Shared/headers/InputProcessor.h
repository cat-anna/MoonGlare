#pragma once

namespace MoonGlare::Core {

using KeyId = uint16_t;
using InputStateId = uint8_t;
static_assert(1 << (sizeof(InputStateId) * 8) >= Configuration::Input::MaxInputStates, "To small InputStateId base type");

union InputStateValue {
    bool m_Boolean;
    float m_Float;
};

struct InputState {
    enum class Type {
        Invalid,
        Switch,
        FloatAxis,
    };

    struct {
        bool m_Valid : 1;
    } m_Flags;
    InputStateValue m_Value;
    Type m_Type;
    unsigned m_ActiveKeyId;
    Configuration::RuntimeRevision m_Revision;
};

struct KeyAction {
    struct {
        bool m_Valid : 1;
        bool m_Positive : 1;
    } m_Flags;
    InputStateId m_Id;
    InputStateValue m_Value;
};

enum class MouseAxisId {
    X,
    Y,
    ScrollX,
    ScrollY,
    Unknown,
    MaxValue = Unknown,
};
static_assert(static_cast<int>(MouseAxisId::MaxValue) == Configuration::Input::MaxMouseAxes, "Invalid mouse axes settings!");

struct AxisAction {
    struct {
        bool m_Valid : 1;
    } m_Flags;
    InputStateId m_Id;
    float m_Sensitivity;
};

struct InputKeyOffsets {
    enum e : unsigned {
        Keyboard = 0,
        Mouse = Configuration::Input::MaxKeyCode - Configuration::Input::MaxMouseButton,
    };
};

enum class InputSwitchState {
    Off,
    Pressed,
    On,
    Released,
};

} //namespace MoonGlare::Core

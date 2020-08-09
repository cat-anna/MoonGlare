#pragma once

namespace MoonGlare::Configuration::Input {

enum {
    MaxKeyCode = 512,
    MaxInputStates = 64,
    MaxMouseButton = 8,
    MaxMouseAxes = 4,
};
static const float StaticMouseSensivity = 0.005f;
static const float StaticMouseScrollSensivity = 1.0f;
static const char *SettingsFileName = "InputSettings.xml";

}

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

//---------------------------------------------------------------------------------------

struct KeyNamesTable {
    KeyNamesTable() {
#define set(V, N) m_Array[V] = N
        set(GLFW_KEY_0, "0");
        set(GLFW_KEY_1, "1");
        set(GLFW_KEY_2, "2");
        set(GLFW_KEY_3, "3");
        set(GLFW_KEY_4, "4");
        set(GLFW_KEY_5, "5");
        set(GLFW_KEY_6, "6");
        set(GLFW_KEY_7, "7");
        set(GLFW_KEY_8, "8");
        set(GLFW_KEY_9, "9");
        set(GLFW_KEY_A, "A");
        set(GLFW_KEY_B, "B");
        set(GLFW_KEY_C, "C");
        set(GLFW_KEY_D, "D");
        set(GLFW_KEY_E, "E");
        set(GLFW_KEY_F, "F");
        set(GLFW_KEY_G, "G");
        set(GLFW_KEY_H, "H");
        set(GLFW_KEY_I, "I");
        set(GLFW_KEY_J, "J");
        set(GLFW_KEY_K, "K");
        set(GLFW_KEY_L, "L");
        set(GLFW_KEY_M, "M");
        set(GLFW_KEY_N, "N");
        set(GLFW_KEY_O, "O");
        set(GLFW_KEY_P, "P");
        set(GLFW_KEY_Q, "Q");
        set(GLFW_KEY_R, "R");
        set(GLFW_KEY_S, "S");
        set(GLFW_KEY_T, "T");
        set(GLFW_KEY_U, "U");
        set(GLFW_KEY_V, "V");
        set(GLFW_KEY_W, "W");
        set(GLFW_KEY_X, "X");
        set(GLFW_KEY_Y, "Y");
        set(GLFW_KEY_Z, "Z");
        set(GLFW_KEY_F1, "F1");
        set(GLFW_KEY_F2, "F2");
        set(GLFW_KEY_F3, "F3");
        set(GLFW_KEY_F4, "F4");
        set(GLFW_KEY_F5, "F5");
        set(GLFW_KEY_F6, "F6");
        set(GLFW_KEY_F7, "F7");
        set(GLFW_KEY_F8, "F8");
        set(GLFW_KEY_F9, "F9");
        set(GLFW_KEY_F10, "F10");
        set(GLFW_KEY_F11, "F11");
        set(GLFW_KEY_F12, "F12");

        set(GLFW_KEY_SPACE, " ");
        set(GLFW_KEY_APOSTROPHE, "'");
        set(GLFW_KEY_COMMA, ",");
        set(GLFW_KEY_MINUS, "-");
        set(GLFW_KEY_PERIOD, ".");
        set(GLFW_KEY_SLASH, "/");
        set(GLFW_KEY_SEMICOLON, ";");
        set(GLFW_KEY_EQUAL, "=");
        set(GLFW_KEY_LEFT_BRACKET, "[");
        set(GLFW_KEY_BACKSLASH, "\\");
        set(GLFW_KEY_RIGHT_BRACKET, "]");
        set(GLFW_KEY_GRAVE_ACCENT, "`");
        set(GLFW_KEY_LEFT_SHIFT, "Left shift");
        set(GLFW_KEY_LEFT_CONTROL, "Left control");
        set(GLFW_KEY_LEFT_ALT, "Left alt");
        set(GLFW_KEY_LEFT_SUPER, "Left super");
        set(GLFW_KEY_RIGHT_SHIFT, "Right shift");
        set(GLFW_KEY_RIGHT_CONTROL, "Right control");
        set(GLFW_KEY_RIGHT_ALT, "Right alt");
        set(GLFW_KEY_RIGHT_SUPER, "Right super");
        set(GLFW_KEY_ESCAPE, "Escape");
        set(GLFW_KEY_ENTER, "Enter");
        set(GLFW_KEY_TAB, "Tab");
        set(GLFW_KEY_BACKSPACE, "Backspace");
        set(GLFW_KEY_INSERT, "Insert");
        set(GLFW_KEY_DELETE, "Delete");
        set(GLFW_KEY_RIGHT, "Right");
        set(GLFW_KEY_LEFT, "Left");
        set(GLFW_KEY_DOWN, "Down");
        set(GLFW_KEY_UP, "Up");
        set(GLFW_KEY_PAGE_UP, "Page Up");
        set(GLFW_KEY_PAGE_DOWN, "Page Down");
        set(GLFW_KEY_HOME, "Home");
        set(GLFW_KEY_END, "End");
        set(GLFW_KEY_CAPS_LOCK, "Caps lock");
        set(GLFW_KEY_SCROLL_LOCK, "Scroll lock");
        set(GLFW_KEY_NUM_LOCK, "Num lock");
        set(GLFW_KEY_PRINT_SCREEN, "Print screen");
        set(GLFW_KEY_PAUSE, "Pause");
        set(GLFW_KEY_KP_0, "Numpad 0");
        set(GLFW_KEY_KP_1, "Numpad 1");
        set(GLFW_KEY_KP_2, "Numpad 2");
        set(GLFW_KEY_KP_3, "Numpad 3");
        set(GLFW_KEY_KP_4, "Numpad 4");
        set(GLFW_KEY_KP_5, "Numpad 5");
        set(GLFW_KEY_KP_6, "Numpad 6");
        set(GLFW_KEY_KP_7, "Numpad 7");
        set(GLFW_KEY_KP_8, "Numpad 8");
        set(GLFW_KEY_KP_9, "Numpad 9");
        set(GLFW_KEY_KP_DECIMAL, "Numpad dot");
        set(GLFW_KEY_KP_DIVIDE, "Numpad divide");
        set(GLFW_KEY_KP_MULTIPLY, "Numpad multiply");
        set(GLFW_KEY_KP_SUBTRACT, "Numpad subtrack");
        set(GLFW_KEY_KP_ADD, "Numpad add");
        set(GLFW_KEY_KP_ENTER, "Numpad enter");
        set(GLFW_KEY_KP_EQUAL, "Numpad equal");
        set(GLFW_KEY_MENU, "Menu");
        set(GLFW_KEY_WORLD_1, "World 1");
        set(GLFW_KEY_WORLD_2, "World 2");
#undef set
#define set_mouse(V, N) m_Array[InputKeyOffsets::Mouse + V] = N
        set_mouse(GLFW_MOUSE_BUTTON_1, "Mouse button Left");
        set_mouse(GLFW_MOUSE_BUTTON_2, "Mouse button Right");
        set_mouse(GLFW_MOUSE_BUTTON_3, "Mouse button Middle");
        set_mouse(GLFW_MOUSE_BUTTON_4, "Mouse button 4");
        set_mouse(GLFW_MOUSE_BUTTON_5, "Mouse button 5");
        set_mouse(GLFW_MOUSE_BUTTON_6, "Mouse button 6");
        set_mouse(GLFW_MOUSE_BUTTON_7, "Mouse button 7");
        set_mouse(GLFW_MOUSE_BUTTON_8, "Mouse button 8");
#undef set_mouse
    }
    const char *Get(KeyId kid) const {
        if (kid >= m_Array.size() || !m_Array[kid])
            return "Unknown key";
        return m_Array[kid];
    }
    bool Find(const char *Name, KeyId &out) const {
        for (KeyId kid = 0; kid < m_Array.size(); ++kid)
            if (m_Array[kid] && stricmp(Name, m_Array[kid]) == 0)
                return out = kid, true;
        return false;
    }

    const char *Get(MouseAxisId maid) const {
        switch (maid)
        {
        case MouseAxisId::X:
            return "Mouse axis X";
        case MouseAxisId::Y:
            return "Mouse axis Y";
        case MouseAxisId::ScrollX:
            return "Mouse scroll X";
        case MouseAxisId::ScrollY:
            return "Mouse scroll Y";
        default:
            return "Unknown mouse axis";
        }
    }

    template<typename T>
    const char *operator[](T t) const { return Get(t); }

    auto& GetTable() const { return m_Array; }
private:
    std::array<const char *, Configuration::Input::MaxKeyCode> m_Array;
};

extern const KeyNamesTable g_KeyNamesTable;

} //namespace MoonGlare::Core

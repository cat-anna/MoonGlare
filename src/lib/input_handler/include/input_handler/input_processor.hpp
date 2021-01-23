#pragma once

#include "input_handler.hpp"
#include <stop_interface.hpp>

namespace MoonGlare::InputHandler {

class InputProcessor final : public iInputHandler {
public:
    InputProcessor(iStopInterface *stop_interface);
    ~InputProcessor();
#if 0
    bool Initialize(World *world);
    bool Finalize();
    void SetInputSource(Renderer::iContext *Context) { m_Context = Context; }

    bool Step(const Core::MoveConfig &config);

    void PushCharModeKey(unsigned Key, bool Pressed);

    void SetKeyState(unsigned KeyCode, bool Pressed);
    void SetMouseButtonState(unsigned Button, bool Pressed) {
        ProcessKeyState(Button + InputKeyOffsets::Mouse, Pressed);
    }
    void SetMouseDelta(const emath::fvec2 &delta) {
        ProcessMouseAxis(MouseAxisId::X, delta.x());
        ProcessMouseAxis(MouseAxisId::Y, delta.y());
    }
    void SetMouseScrollDelta(const emath::fvec2 &delta) {
        ProcessMouseAxis(MouseAxisId::ScrollX, delta.x());
        ProcessMouseAxis(MouseAxisId::ScrollY, delta.y());
    }
    void ClearStates();

    bool Save(pugi::xml_node node) const;
    bool Load(const pugi::xml_node node);

    void Clear();
    void ResetToInternalDefault();

    bool AddKeyboardAxis(const char *Name, KeyId PositiveKey, KeyId NegativeKey);
    bool AddKeyboardSwitch(const char *Name, KeyId Key);
    bool AddMouseAxis(const char *Name, MouseAxisId axis, float Sensitivity);

    bool RegisterKeySwitch(const char *Name, const char *KeyName);
    bool RegisterKeyboardAxis(const char *Name, const char *PositiveKeyName, const char *NegativeKeyName);

    static void RegisterScriptApi(ApiInitializer &root);

    bool BeginCharMode(Entity reciver);
    void CaptureKey(Entity reciver);
    void EndCharMode();
#endif

    //iInputHandler
    void OnFocusChange(bool State) override;
    bool ShouldClose(bool Focus, bool MouseHook) override;

protected:
    iStopInterface *const stop_interface;
#if 0
    using InputStateArray = Memory::StaticVector<InputState, Configuration::Input::MaxInputStates>;
    using KeyMapArray = std::array<KeyAction, Configuration::Input::MaxKeyCode>;
    using MouseAxesArray = std::array<AxisAction, Configuration::Input::MaxMouseAxes>;

    InputStateArray m_InputStates;
    MouseAxesArray m_MouseAxes;
    KeyMapArray m_Keys;
    Configuration::RuntimeRevision m_CurrentRevision;

    std::unordered_map<std::string, InputStateId> m_InputNames;
    World *m_World = nullptr;
    iConsole *m_Console = nullptr;
    Renderer::iContext *m_Context = nullptr;

    bool m_ConsoleActive = false;
    enum class CharMode {
        None,
        Console,
        ScriptChar,
        ScriptKey,
    };
    CharMode m_CharMode = CharMode::None;
    Entity m_CharReciver = {};

    void ProcessKeyState(unsigned Id, bool Pressed);
    void ProcessMouseAxis(MouseAxisId Id, float Delta);
    bool GetInputStateName(InputStateId isid, std::string &out) const;

    InputState *AllocInputState(InputState::Type type, const std::string &Name, InputStateId &outindex);
    KeyAction *AllocKeyAction(KeyId kid, InputStateId isid, bool Positive);
    AxisAction *AllocMouseAxis(MouseAxisId maid, InputStateId isid, float Sensitivity);

    bool ProcessConsoleActivateKey();

    static int luaIndexInput(lua_State *lua);
#endif
};

} // namespace MoonGlare::InputHandler
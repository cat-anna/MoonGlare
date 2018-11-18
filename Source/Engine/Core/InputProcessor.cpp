#include <pch.h>
#include <nfMoonGlare.h>

#include <Foundation/GLFWKeyMapping.h>

#include "InputProcessor.h"
#include "InputProcessor.Events.h"

#include <Foundation/OS/Path.h>
#include <Core/Scripts/LuaApi.h>
#include <Core/Scripts/ScriptEngine.h>
#include <Core/Scene/ScenesManager.h>

#include <Core/Scripts/ScriptComponent.h>

#include "iConsole.h"

#include <Renderer/iContext.h>
#include <Renderer/iContextInputHandler.h>

#include <Input.x2c.h>

namespace MoonGlare {
namespace Core {

const KeyNamesTable g_KeyNamesTable;

Space::RTTI::TypeInfoInitializer<InputProcessor, InputStateValue, InputState, KeyAction, AxisAction, KeyNamesTable> InputStructures;
RegisterApiNonClass(InputProcessor, &InputProcessor::RegisterScriptApi);

//---------------------------------------------------------------------------------------

InputProcessor::InputProcessor() {
    m_CurrentRevision = 1;
    m_World = nullptr;
    ResetToInternalDefault();
}

InputProcessor::~InputProcessor() {
}

//---------------------------------------------------------------------------------------

void InputProcessor::RegisterScriptApi(ApiInitializer &root) {
    root
        .beginClass<InputProcessor>("cInputProcessor")
            .addFunction("RegisterKeySwitch", &InputProcessor::RegisterKeySwitch)
            .addFunction("RegisterKeyboardAxis", &InputProcessor::RegisterKeyboardAxis)

            .addFunction("BeginCharMode", &InputProcessor::BeginCharMode)
            .addFunction("EndCharMode", &InputProcessor::EndCharMode)
            .addFunction("CaptureKey", &InputProcessor::CaptureKey)
        .endClass()
        ;                    
}

//---------------------------------------------------------------------------------------

bool InputProcessor::Initialize(World *world)  {
    m_World = world;
    auto *ScriptEngine = m_World->GetScriptEngine();
    {
        auto lua = ScriptEngine->GetLua();
        LOCK_MUTEX_NAMED(ScriptEngine->GetLuaMutex(), lock);
        Scripts::LuaStackOverflowAssert check(lua);

        lua_createtable(lua, 0, 0);					// stack: selftable
        MoonGlare::Core::Scripts::PublishSelfLuaTable(lua, "InputProcessor", this, -1);
        lua_pushlightuserdata(lua, (void *)this);	// stack: selftable selfptr
        lua_pushvalue(lua, -2);		  				// stack: selftable selfptr selftable
        lua_settable(lua, LUA_REGISTRYINDEX);		// stack: selftable

        lua_pushlightuserdata(lua, (void *)this);	// stack: selftable selfptr
        lua_pushcclosure(lua, &InputProcessor::luaIndexInput, 1);// stack: selftable cclosure
        lua_setfield(lua, -2, "__index");			// stack: selftable

        lua_createtable(lua, 0, 0);					// stack: selftable InputTable
        lua_insert(lua, -2);						// stack: InputTable selftable 
        lua_setmetatable(lua, -2);					// stack: InputTable
        lua_setglobal(lua, "Input");				// stack:
    }

    ScriptEngine->RegisterLuaSettings(this, "Input");

    pugi::xml_document doc;
    auto fname = OS::GetSettingsFilePath(Configuration::Input::SettingsFileName);
    if (doc.load_file(fname.c_str())) {
        if (!Load(doc.document_element())) {
            AddLog(Error, "Failed to load input configuration");
            return false;
        }
        AddLogf(Info, "Loaded InputConfiguration from: %s", fname.c_str());
    }

    AddKeyboardSwitch("Escape", GLFW_KEY_ESCAPE);

    m_World->GetObject(m_Console);

    return true;
}

bool InputProcessor::Finalize() {
    {
        auto *se = m_World->GetScriptEngine();
        auto lua = se->GetLua();
        LOCK_MUTEX_NAMED(se->GetLuaMutex(), lock);
        Scripts::LuaStackOverflowAssert check(lua);

        lua_pushlightuserdata(lua, (void *)this);
        lua_pushnil(lua);
        lua_settable(lua, LUA_REGISTRYINDEX);

        lua_pushnil(lua);
        lua_setglobal(lua, "Input");	

        MoonGlare::Core::Scripts::HideSelfLuaTable(lua, "InputProcessor", this);
    }
    return true;
}

//---------------------------------------------------------------------------------------

void InputProcessor::CaptureKey(Entity reciver) {
    if (m_CharMode != CharMode::None)
        return;
    m_CharReciver = reciver;
    m_CharMode = CharMode::ScriptKey;
}

bool InputProcessor::BeginCharMode(Entity reciver) {
    if (m_CharMode != CharMode::None)
        return false;

    m_CharReciver = reciver;
    m_Context->EnterCharMode();

    m_CharMode = CharMode::ScriptChar;
    return true;
}

void InputProcessor::EndCharMode() {
    if (m_CharMode == CharMode::ScriptChar) {
        m_Context->ExitCharMode();
        m_CharMode = CharMode::None;
    }
}

//---------------------------------------------------------------------------------------

bool InputProcessor::ProcessConsoleActivateKey() {
    if (!m_Console || !m_Context)
        return false;

    m_ConsoleActive = !m_ConsoleActive;
    if (m_ConsoleActive) {
        m_CharMode = CharMode::Console;
        m_Context->EnterCharMode();
        m_Console->Activate();
        AddLogf(Debug, "Console activated");
    } else {
        m_CharMode = CharMode::None;
        m_Context->ExitCharMode();
        m_Console->Deactivate();
        AddLogf(Debug, "Console deactivated");
    }

    return true;
}

void InputProcessor::PushCharModeKey(unsigned key, bool Pressed) {
    if (m_CharMode == CharMode::None || !Pressed)
        return;

    //if (m_CharMode == CharMode::ScriptChar) {
    //    auto scene = m_World->GetScenesManager()->CurrentScene();
    //    auto &cm = scene->GetSubsystemManager();
    //    auto &ed = cm.GetEventDispatcher();
    //    ed.Send(InputProcessorOnCharEvent{ m_CharReciver, (int)key } );
    //    return;
    //}

    if (m_ConsoleActive && m_Console) {
        using Key = KeyMapping;
        Key k = (Key) key;
        if (k == Key::Escape) {
            ProcessConsoleActivateKey();
            return;
        }
        m_Console->PushKey(key);
    }
}

//---------------------------------------------------------------------------------------

bool InputProcessor::Step(const Core::MoveConfig & config) {
    ++m_CurrentRevision;
    return true;
}

//---------------------------------------------------------------------------------------

void InputProcessor::SetKeyState(unsigned KeyCode, bool Pressed) {
    //if (m_CharMode == CharMode::ScriptKey) {
    //    if (Pressed) {
    //        m_CharMode = CharMode::None;
    //        auto scene = m_World->GetScenesManager()->CurrentScene();
    //        auto &cm = scene->GetSubsystemManager();
    //        auto &ed = cm.GetEventDispatcher();
    //        ed.Send(InputProcessorOnKeyEvent{ m_CharReciver, (int)KeyCode } );
    //    }
    //    return;
    //}
    switch (KeyCode) {
    case Configuration::Console::ActivateKey:
        if (Pressed || ProcessConsoleActivateKey())
            break;
    default:
        ProcessKeyState(KeyCode + InputKeyOffsets::Keyboard, Pressed);
    }
}

void InputProcessor::ProcessKeyState(unsigned Id, bool Pressed) {
    if (Id >= Configuration::Input::MaxKeyCode) {
        AddLogf(Warning, "Key code id overflow: %u", Id);
        return;
    }
    //AddLogf(Debug, "Processing key: %u:%d", Id, Pressed ? 1 : 0);

    auto &keyinfo = m_Keys[Id];
    if (!keyinfo.m_Flags.m_Valid) 
        return;
    
    auto &state = m_InputStates[keyinfo.m_Id];

    //AddLogf(Debug, "State:%u currkey:%u value:%f", Id, state.m_ActiveKeyId, state.m_Value.m_Float);

    if (Pressed || state.m_ActiveKeyId == Id || state.m_ActiveKeyId == Configuration::Input::MaxKeyCode) { 
        //while handling key release do not update state if it is changed by other key
        switch (state.m_Type) {
        case InputState::Type::Switch:
            state.m_Value.m_Boolean = Pressed;
            break;
        case InputState::Type::FloatAxis:
            state.m_Value.m_Float = Pressed ? keyinfo.m_Value.m_Float : 0.0f;
            break;
        default:
            LogInvalidEnum(state.m_Type);
            return;
        }
    }
    state.m_Revision = m_CurrentRevision + 1;
    state.m_ActiveKeyId = Pressed ? Id : Configuration::Input::MaxKeyCode;
}

void InputProcessor::ProcessMouseAxis(MouseAxisId Id, float Delta) {
    THROW_ASSERT(Id <MouseAxisId::Unknown, "Mouse axis id overflow!");
    //AddLogf(Debug, "Processing mouse axis: %d:%f", Id, Delta);

    auto &axis = m_MouseAxes[static_cast<size_t>(Id)];
    if (!axis.m_Flags.m_Valid)
        return;
    auto &state = m_InputStates[axis.m_Id];
    if (!state.m_Flags.m_Valid || state.m_Type != InputState::Type::FloatAxis) {
        AddLog(Error, "Invalid Input state or invalid state type!");
        return;
    }
    
    state.m_Value.m_Float = Delta * axis.m_Sensitivity;
}

void InputProcessor::ClearStates() {
    for (auto &it : m_MouseAxes) {
        if (!it.m_Flags.m_Valid)
            continue;
        m_InputStates[it.m_Id].m_Value.m_Float = 0.0f;
    }
}

//---------------------------------------------------------------------------------------

bool InputProcessor::Save(pugi::xml_node node) const {
    x2c::Core::Input::InputConfiguration_t conf;
    for (auto &it : m_MouseAxes) {
        if (!it.m_Flags.m_Valid)
            continue;

        x2c::Core::Input::MouseAxis_t ma;
        ma.m_AxisId = static_cast<MouseAxisId>(&it - &m_MouseAxes[0]);
        ma.m_Sensitivity = it.m_Sensitivity;
        switch (ma.m_AxisId) {
        case MouseAxisId::X:
        case MouseAxisId::Y:
            ma.m_Sensitivity /= Configuration::Input::StaticMouseSensivity;
            break;
        case MouseAxisId::ScrollX:
        case MouseAxisId::ScrollY:
            ma.m_Sensitivity /= Configuration::Input::StaticMouseScrollSensivity;
            break;
        default:
            LogInvalidEnum(ma.m_AxisId);
            break;
        }
        GetInputStateName(it.m_Id, ma.m_Name);
        conf.m_MouseAxes.push_back(std::move(ma));
    }

    std::unordered_map<InputStateId, std::list<const KeyAction*>> KeyActionsMap;

    for (auto &it : m_Keys) {
        if (!it.m_Flags.m_Valid)
            continue;
        KeyActionsMap[it.m_Id].push_back(&it);
    }

    for (auto &it : KeyActionsMap) {
        auto &list = it.second;
        auto &state = m_InputStates[it.first];

        if (!state.m_Flags.m_Valid) {
            //just to be sure
            continue;
        }

        switch (state.m_Type) {
        case InputState::Type::Switch: {
            x2c::Core::Input::KeyboardSwitch_t sw;
            GetInputStateName(it.first, sw.m_Name);
            for (auto key : list) {
                auto index = static_cast<KeyId>(key - &m_Keys[0]);
                sw.m_Keys.push_back(index);
            }
            conf.m_KeyboardSwitches.push_back(std::move(sw));
            break;
        }
        case InputState::Type::FloatAxis:{
            x2c::Core::Input::KeyboardAxis_t ka;
            GetInputStateName(it.first, ka.m_Name);
            for (auto key : list) {
                auto index = static_cast<KeyId>(key - &m_Keys[0]);
                if (key->m_Flags.m_Positive)
                    ka.m_NegativeKeys.push_back(index);
                else
                    ka.m_PositiveKeys.push_back(index);
            }
            conf.m_KeyboardAxes.push_back(std::move(ka));
            break;
        }
        default:
            LogInvalidEnum(state.m_Type);
            continue;
        }
    }

    return conf.Write(node);
}

bool InputProcessor::Load(const pugi::xml_node node) {
    Clear();
    x2c::Core::Input::InputConfiguration_t conf;
    if (!conf.Read(node)) {
        AddLog(Error, "Failed to read input configuration!");
        return false;
    }

    for (auto &it : conf.m_KeyboardSwitches) {
        InputStateId InputIndex;
        if (!AllocInputState(InputState::Type::Switch, it.m_Name, InputIndex)) {
            //no need for more logging
            return false;
        }
        for (auto key : it.m_Keys) {
            AllocKeyAction(key, InputIndex, true);
        }
    }

    for (auto &it : conf.m_KeyboardAxes) {
        InputStateId InputIndex;
        if (!AllocInputState(InputState::Type::FloatAxis, it.m_Name, InputIndex)) {
            //no need for more logging
            return false;
        }
        for (auto key : it.m_PositiveKeys) {
            AllocKeyAction(key, InputIndex, true);
        }
        for (auto key : it.m_NegativeKeys) {
            AllocKeyAction(key, InputIndex, false);
        }
    }

    for (auto &it : conf.m_MouseAxes) {
        AddMouseAxis(it.m_Name.c_str(), it.m_AxisId, it.m_Sensitivity);
    }

    AddLog(Info, "Loaded input configuration");
    return true;
}

void InputProcessor::Clear() {
    m_InputStates.MemZeroAndClear();
    Space::MemZero(m_MouseAxes);
    Space::MemZero(m_Keys);
    m_InputNames.clear();
}

void InputProcessor::ResetToInternalDefault() {
    Clear();

    //AddKeyboardAxis("Forward", GLFW_KEY_UP, GLFW_KEY_DOWN);
    //AddKeyboardAxis("SideStep", GLFW_KEY_LEFT, GLFW_KEY_RIGHT);

    AddKeyboardAxis("Forward", GLFW_KEY_W, GLFW_KEY_S);
    AddKeyboardAxis("SideStep", GLFW_KEY_A, GLFW_KEY_D);

    AddKeyboardSwitch("Run", GLFW_KEY_LEFT_SHIFT);

    AddMouseAxis("Turn", MouseAxisId::X, 0.5f);
    AddMouseAxis("LookAngle", MouseAxisId::Y, 0.5f);
}

bool InputProcessor::GetInputStateName(InputStateId isid, std::string &out) const {
    for(auto &it: m_InputNames)
        if (it.second == isid) {
            out = it.first;
            return true;
        }
    out = "?";
    return false;
}

//---------------------------------------------------------------------------------------

InputState* InputProcessor::AllocInputState(InputState::Type type, const std::string &Name, InputStateId &outindex) {
    InputState *stateptr = nullptr;

    auto it = m_InputNames.find(Name);
    if (it != m_InputNames.end()) {
        stateptr = &m_InputStates[it->second];
        outindex = static_cast<InputStateId>(it->second);
        if (!stateptr->m_Flags.m_Valid) {
            AddLogf(Error, "There was a invalid input state: %s", Name.c_str());
        } else {
            if (stateptr->m_Type != type) {
                AddLogf(Error, "Attempt to redefine type of input state: %s", Name.c_str());
                return false;
            }
        }
    } else {
        size_t idx;
        if (!m_InputStates.Allocate(idx)) {
            AddLog(Error, "No more space to add keyboard axis");
            return nullptr;
        }
        outindex = static_cast<InputStateId>(idx);
        m_InputNames[Name] = static_cast<InputStateId>(outindex);
        stateptr = &m_InputStates[outindex];
    }

    auto &state = *stateptr;
    state.m_Flags.m_Valid = true;
    state.m_Type = type;

    switch (state.m_Type) {
    case InputState::Type::Switch:
        state.m_Value.m_Boolean = false;
        break;
    case InputState::Type::FloatAxis:
        state.m_Value.m_Float = 0.0f;
        break;
    default:
        LogInvalidEnum(state.m_Type);
        break;
    }

    return &state;
}

KeyAction* InputProcessor::AllocKeyAction(KeyId kid, InputStateId isid, bool Positive) {
    THROW_ASSERT(kid < Configuration::Input::MaxKeyCode, "KeyId overflow!");
    auto &key = m_Keys[kid];
    key.m_Flags.m_Valid = true;
    key.m_Flags.m_Positive = Positive;
    key.m_Id = isid;

    auto &state = m_InputStates[isid];

    switch (state.m_Type) {
    case InputState::Type::Switch:
        key.m_Value.m_Boolean = Positive;
        break;
    case InputState::Type::FloatAxis:
        key.m_Value.m_Float = Positive ? 1.0f : -1.0f;
        break;
    default:
        LogInvalidEnum(state.m_Type);
        break;
    }
    return &key;
}

AxisAction* InputProcessor::AllocMouseAxis(MouseAxisId maid, InputStateId isid, float Sensitivity) {
    auto &axis = m_MouseAxes[static_cast<size_t>(maid)];
    axis.m_Flags.m_Valid = true;
    axis.m_Sensitivity = Sensitivity;
    axis.m_Id = isid;

    switch (maid) {
    case MouseAxisId::X:
    case MouseAxisId::Y:
        axis.m_Sensitivity *= Configuration::Input::StaticMouseSensivity;
        break;
    case MouseAxisId::ScrollX:
    case MouseAxisId::ScrollY:
        axis.m_Sensitivity *= Configuration::Input::StaticMouseScrollSensivity;
        break;
    default:
        LogInvalidEnum(maid);
        break;
    }
    return &axis;
}

bool InputProcessor::AddKeyboardAxis(const char *Name, KeyId PositiveKey, KeyId NegativeKey) {
    THROW_ASSERT(PositiveKey < Configuration::Input::MaxKeyCode, "PositiveKey id overflow!");
    THROW_ASSERT(NegativeKey < Configuration::Input::MaxKeyCode, "NegativeKey id overflow!");

    InputStateId InputIndex;
    if (!AllocInputState(InputState::Type::FloatAxis, Name, InputIndex)) {
        //no need for more logging
        return false;
    }

    AllocKeyAction(PositiveKey, InputIndex, true);
    AllocKeyAction(NegativeKey, InputIndex, false);

    return true;
}

bool InputProcessor::AddKeyboardSwitch(const char *Name, KeyId Key) {
    THROW_ASSERT(Key < Configuration::Input::MaxKeyCode, "Key id overflow!");

    InputStateId InputIndex;
    if (!AllocInputState(InputState::Type::Switch, Name, InputIndex)) {
        //no need for more logging
        return false;
    }
    AllocKeyAction(Key, InputIndex, true);

    return true;
}

bool InputProcessor::AddMouseAxis(const char *Name, MouseAxisId axisid, float Sensitivity) {
    THROW_ASSERT(axisid < MouseAxisId::Unknown, "MouseAxisId overflow!");

    InputStateId InputIndex;
    if (!AllocInputState(InputState::Type::FloatAxis, Name, InputIndex)) {
        //no need for more logging
        return false;
    }
    AllocMouseAxis(axisid, InputIndex, Sensitivity);

    return true;
}

bool InputProcessor::RegisterKeySwitch(const char *Name, const char *KeyName) {
    KeyId kid;
    if (!g_KeyNamesTable.Find(KeyName, kid)) {
        AddLogf(Warning, "Unknown key: %s", KeyName);
        return false;
    }

    return AddKeyboardSwitch(Name, kid);
}

bool InputProcessor::RegisterKeyboardAxis(const char * Name, const char *PositiveKeyName, const char *NegativeKeyName) {
    KeyId Positivekid;
    if (!g_KeyNamesTable.Find(PositiveKeyName, Positivekid)) {
        AddLogf(Warning, "Unknown key: %s", PositiveKeyName);
        return false;
    }
    KeyId Negativekid;
    if (!g_KeyNamesTable.Find(NegativeKeyName, Negativekid)) {
        AddLogf(Warning, "Unknown key: %s", NegativeKeyName);
        return false;
    }

    return AddKeyboardAxis(Name, Positivekid, Negativekid);
}

//---------------------------------------------------------------------------------------

int InputProcessor::luaIndexInput(lua_State *lua) {
    void *ThisPtr = lua_touserdata(lua, lua_upvalueindex(1));
    InputProcessor *This = reinterpret_cast<InputProcessor*>(ThisPtr);

    const char *name = lua_tostring(lua, -1);

    auto it = This->m_InputNames.find(name);
    if (it == This->m_InputNames.end()) {
        return 0;
    }

    auto &state = This->m_InputStates[it->second];
    if (!state.m_Flags.m_Valid) {
        AddLogf(Error, "Found invalid input state: %s", name);
        return 0;
    }

    switch (state.m_Type) {
    case InputState::Type::Switch:
        if (!state.m_Value.m_Boolean) {
            if (state.m_Revision == This->m_CurrentRevision) {
                lua_pushinteger(lua, (int)InputSwitchState::Released);
            } else {
                lua_pushinteger(lua, (int)InputSwitchState::Off);
            }
        } else {
            if (state.m_Revision == This->m_CurrentRevision) {
                lua_pushinteger(lua, (int)InputSwitchState::Pressed);
            } else {
                lua_pushinteger(lua, (int)InputSwitchState::On);
            }
        }
        return 1;
    case InputState::Type::FloatAxis:
        lua_pushnumber(lua, state.m_Value.m_Float);
        return 1;
    default:
        LogInvalidEnum(state.m_Type);
        return 0;
    }
}

//---------------------------------------------------------------------------------------

void InputProcessor::OnKey(int Key, bool Pressed) {
    SetKeyState(Key, Pressed);
}

void InputProcessor::OnChar(unsigned CharOrKey, bool Pressed) {
    PushCharModeKey(CharOrKey, Pressed);
}

void InputProcessor::OnScroll(const emath::fvec2 & delta) {
    SetMouseScrollDelta(delta);
}

void InputProcessor::OnMouseButton(int Button, bool State) {
    ProcessKeyState(Button + InputKeyOffsets::Mouse, State);
}

void InputProcessor::MouseDelta(const emath::fvec2 & delta) {
    SetMouseDelta(delta);
}

bool InputProcessor::CanDropFocus() {
    return DEBUG_TRUE;
}

void InputProcessor::OnFocusChange(bool State) {
}

bool InputProcessor::CanReleaseMouse() {
    return DEBUG_TRUE;
}

void InputProcessor::OnMouseHookChange(bool State) {
}

bool InputProcessor::ShouldClose(bool Focus, bool MouseHook) {
    return true;
}

} //namespace Core 
} //namespace MoonGlare 

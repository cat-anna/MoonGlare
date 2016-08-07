/*
  * Generated by cppsrc.sh
  * On 2016-08-04 19:45:53,67
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "InputProcessor.h"

//#include <Input.x2c.h>

namespace MoonGlare {
namespace Core {

RegisterApiNonClass(InputProcessor, &InputProcessor::RegisterScriptApi, "Input");

InputProcessor::InputProcessor() {
	m_World = nullptr;
	ResetToInternalDefault();
}

InputProcessor::~InputProcessor() {

}

//---------------------------------------------------------------------------------------

void InputProcessor::RegisterScriptApi(ApiInitializer &root) {

	//	root
	//		.addCFunction()

}

//---------------------------------------------------------------------------------------

bool InputProcessor::Initialize(World *world) {
	m_World = world;
	{
		auto *se = m_World->GetScriptEngine();
		auto lua = se->GetLua();
		LOCK_MUTEX_NAMED(se->GetLuaMutex(), lock);
		Utils::Scripts::LuaStackOverflowAssert check(lua);

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
	return true;
}

bool InputProcessor::Finalize() {
	{
		auto *se = m_World->GetScriptEngine();
		auto lua = se->GetLua();
		LOCK_MUTEX_NAMED(se->GetLuaMutex(), lock);
		Utils::Scripts::LuaStackOverflowAssert check(lua);

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

void InputProcessor::ProcessKeyState(unsigned Id, bool Pressed) {
	THROW_ASSERT(Id < Configuration::Input::MaxKeyCode, "Key code id overflow!");
	//AddLogf(Debug, "Processing key: %u:%d", Id, Pressed ? 1 : 0);

	auto &keyinfo = m_Keys[Id];
	if (!keyinfo.m_Flags.m_Valid) 
		return;
	
	auto &state = m_InputStates[keyinfo.m_Id];

	//AddLogf(Debug, "State:%u currkey:%u value:%f", Id, state.m_ActiveKeyId, state.m_Value.m_Float);

	if (Pressed || state.m_ActiveKeyId == Id || state.m_ActiveKeyId == Configuration::Input::MaxKeyCode) { 
		//while handling key release do not update state if it is changed by other key
		switch (state.m_Type) {
		case InputState::Type::BoolSwitch:
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
	
	state.m_Value.m_Float = Delta * axis.m_Sensivity;
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
	return false;
}

bool InputProcessor::Load(const pugi::xml_node node) {
	return false;
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
	//AddKeyboardAxis("LeftRight", GLFW_KEY_LEFT, GLFW_KEY_RIGHT);

	AddKeyboardAxis("Forward", GLFW_KEY_U, GLFW_KEY_J);
	AddKeyboardAxis("LeftRight", GLFW_KEY_LEFT, GLFW_KEY_RIGHT);

	//AddKeyboardAxis("Forward", GLFW_KEY_W, GLFW_KEY_S);
	//AddKeyboardAxis("LeftRight", GLFW_KEY_A, GLFW_KEY_D);

	AddKeyboardSwitch("Run", GLFW_KEY_LEFT_SHIFT);

	AddMouseAxis("MouseX", MouseAxisId::ScrollX, 1.5f);
	AddMouseAxis("MouseY", MouseAxisId::ScrollY, 1.5f);
}

//---------------------------------------------------------------------------------------

bool InputProcessor::AddKeyboardAxis(const char *Name, unsigned ForwardKey, unsigned BackwardKey) {
	THROW_ASSERT(ForwardKey < Configuration::Input::MaxKeyCode, "ForwardKey id overflow!");
	THROW_ASSERT(BackwardKey < Configuration::Input::MaxKeyCode, "BackwardKey id overflow!");

	size_t InputIndex;
	if (!m_InputStates.Allocate(InputIndex)) {
		AddLog(Error, "No more space to add keyboard axis");
		return false;
	}

	auto &state = m_InputStates[InputIndex];
	state.m_Flags.m_Valid = true;
	state.m_Type = InputState::Type::FloatAxis;
	state.m_Value.m_Float = 0.0f;

	auto &Fkey = m_Keys[ForwardKey];
	Fkey.m_Flags.m_Valid = true;
	Fkey.m_Id = static_cast<InputStateId>(InputIndex);
	Fkey.m_Value.m_Float = 1.0f;

	auto &Bkey = m_Keys[BackwardKey];
	Bkey.m_Flags.m_Valid = true;
	Bkey.m_Id = static_cast<InputStateId>(InputIndex);
	Bkey.m_Value.m_Float = -1.0f;

	m_InputNames[Name] = static_cast<InputStateId>(InputIndex);

	return true;
}

bool InputProcessor::AddKeyboardSwitch(const char * Name, unsigned Key) {
	THROW_ASSERT(Key < Configuration::Input::MaxKeyCode, "Key id overflow!");

	size_t InputIndex;
	if (!m_InputStates.Allocate(InputIndex)) {
		AddLog(Error, "No more space to add keyboard axis");
		return false;
	}

	auto &state = m_InputStates[InputIndex];
	state.m_Flags.m_Valid = true;
	state.m_Type = InputState::Type::BoolSwitch;
	state.m_Value.m_Boolean = false;

	auto &Fkey = m_Keys[Key];
	Fkey.m_Flags.m_Valid = true;
	Fkey.m_Id = static_cast<InputStateId>(InputIndex);
	Fkey.m_Value.m_Boolean = true;

	m_InputNames[Name] = static_cast<InputStateId>(InputIndex);

	return true;
}

bool InputProcessor::AddMouseAxis(const char *Name, MouseAxisId axisid, float Sensivity) {
	THROW_ASSERT(axisid < MouseAxisId::Unknown, "MouseAxisId overflow!");

	size_t InputIndex;
	if (!m_InputStates.Allocate(InputIndex)) {
		AddLog(Error, "No more space to add keyboard axis");
		return false;
	}

	auto &state = m_InputStates[InputIndex];
	state.m_Flags.m_Valid = true;
	state.m_Type = InputState::Type::FloatAxis;
	state.m_Value.m_Float = 0.0f;

	auto &axis = m_MouseAxes[static_cast<size_t>(axisid)];
	axis.m_Flags.m_Valid = true;
	axis.m_Sensivity = Sensivity;
	axis.m_Id = static_cast<InputStateId>(InputIndex);

	switch (axisid) {
	case MouseAxisId::X:
	case MouseAxisId::Y:
		axis.m_Sensivity *= Configuration::Input::StaticMouseSensivity;
		break;
	case MouseAxisId::ScrollX:
	case MouseAxisId::ScrollY:
		axis.m_Sensivity *= Configuration::Input::StaticMouseScrollSensivity;
		break;
	default:
		//wont happen
		break;
	}
	
	m_InputNames[Name] = static_cast<InputStateId>(InputIndex);

	return true;
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
	case InputState::Type::BoolSwitch:
		lua_pushboolean(lua, state.m_Value.m_Boolean);
		return 1;
	case InputState::Type::FloatAxis:
		lua_pushnumber(lua, state.m_Value.m_Float);
		return 1;
	default:
		LogInvalidEnum(state.m_Type);
		return 0;
	}
}

} //namespace Core 
} //namespace MoonGlare 
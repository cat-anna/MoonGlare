/*
  * Generated by cppsrc.sh
  * On 2016-08-04 19:45:53,67
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InputProcessor_H
#define InputProcessor_H

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare {
namespace Core {

using InputStateId = uint8_t;
static_assert(1 << (sizeof(InputStateId) * 8) >= Configuration::Input::MaxInputStates, "To small InputStateId base type");

union InputStateValue {
	bool m_Boolean;
	float m_Float;
};

struct InputState {
	enum class Type {
		Invalid,
		BoolSwitch,
		FloatAxis,
	};

	struct {
		bool m_Valid : 1;
	} m_Flags;
	InputStateValue m_Value;
	Type m_Type;
	unsigned m_ActiveKeyId;
};

struct KeyAction {
	struct {
		bool m_Valid : 1;
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
	float m_Sensivity;
};

struct InputKeyOffsets {
	enum e : unsigned {
		Keyboard = 0,
		Mouse = Configuration::Input::MaxKeyCode - Configuration::Input::MaxMouseButton,
	};
};

//-----

class InputProcessor final {
public:
 	InputProcessor();
 	~InputProcessor();

	bool Initialize(World *world);
	bool Finalize();

	void SetKeyState(unsigned KeyCode, bool Pressed) {
		ProcessKeyState(KeyCode + InputKeyOffsets::Keyboard, Pressed);
	}
	void SetMouseButtonState(unsigned Button, bool Pressed) {
		ProcessKeyState(Button + InputKeyOffsets::Mouse, Pressed);
	}
	void SetMouseDelta(const math::vec2 &delta) {
		ProcessMouseAxis(MouseAxisId::X, delta[0]);
		ProcessMouseAxis(MouseAxisId::Y, delta[1]);
	}
	void SetMouseScrollDelta(const math::vec2 &delta) {
		ProcessMouseAxis(MouseAxisId::ScrollX, delta[0]);
		ProcessMouseAxis(MouseAxisId::ScrollY, delta[1]);
	}
	void ClearStates();

	bool Save(pugi::xml_node node) const;
	bool Load(const pugi::xml_node node);

	void Clear();
	void ResetToInternalDefault();

	bool AddKeyboardAxis(const char *Name, unsigned ForwardKey, unsigned BackwardKey);
	bool AddKeyboardSwitch(const char *Name, unsigned Key);
	bool AddMouseAxis(const char *Name, MouseAxisId axis, float Sensivity);

	static void RegisterScriptApi(ApiInitializer &root);
protected:
	using InputStateArray = Space::Container::StaticVector<InputState, Configuration::Input::MaxInputStates>;
	using KeyMapArray = std::array<KeyAction, Configuration::Input::MaxKeyCode>;
	using MouseAxesArray = std::array<AxisAction, Configuration::Input::MaxMouseAxes>;

	InputStateArray m_InputStates;
	MouseAxesArray m_MouseAxes;
	KeyMapArray m_Keys;

	std::unordered_map<std::string, InputStateId> m_InputNames;
	World *m_World;

	void ProcessKeyState(unsigned Id, bool Pressed);
	void ProcessMouseAxis(MouseAxisId Id, float Delta);
private:
	static int luaIndexInput(lua_State *lua);
};

} //namespace Core 
} //namespace MoonGlare 

#endif
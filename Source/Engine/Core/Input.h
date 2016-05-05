/*
	Generated by cppsrc.sh
	On 2014-12-25 23:39:59,76
	by Paweu
*/

#pragma once
#ifndef Input_H
#define Input_H

namespace Core {

class KeyMap : public cRootClass {
	SPACERTTI_DECLARE_CLASS(KeyMap, cRootClass)
public:
	uint64_t Querry(uint32_t index) const {
		if(index >= MapSize) return 0;
		return m_KeyMap[index];
	}
	void SetValue(uint32_t index, uint64_t value) {
		if(index < MapSize) m_KeyMap[index] = value;
	}
	void AddValue(uint32_t index, uint64_t value) {
		if(index < MapSize) m_KeyMap[index] |= value;
	}
	uint64_t& operator[](uint32_t index) {
		if(index >= MapSize) throw false;
		return m_KeyMap[index];
	}

	unsigned QuerryTableSize() const { return MapSize; }
	void Clear() { memset(m_KeyMap, 0, MapSize * sizeof(m_KeyMap[0])); };
	void ResetKeyMap();
protected:
	enum { MapSize = 512, };
	uint64_t m_KeyMap[MapSize];
};

struct MouseEventDispatcher {
	using MouseButton = ::Graphic::WindowInput::MouseButton;
	using ModsStatus = ::Graphic::WindowInput::ModsStatus;

	virtual ~MouseEventDispatcher() { Revoke(); }
	virtual void MouseUpEvent(MouseButton Button, ModsStatus Modificators) { };
	virtual void MouseDownEvent(MouseButton Button, ModsStatus Modificators) { };

	void Set();
	void Revoke();
};

class Input : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(Input, cRootClass)
public:
 	Input();
 	virtual ~Input();
	bool Initialize();
	bool Finalize();

	void Clear() { ClearKeys(); ClearMouseDelta(); }

	void ClearKeys() { m_KeyStatus = 0; }
	void KeyChange(uint32_t key, bool State){
		auto flag = m_KeyMap.Querry(key);
		if (!flag) return;
		if (State) m_KeyStatus |= flag;
		else m_KeyStatus &= ~flag;
	}

	static void MouseDownEvent(MouseEventDispatcher::MouseButton Button, MouseEventDispatcher::ModsStatus Modificators) {
		if (!_MouseDispatcher)
			return;
		_MouseDispatcher->MouseDownEvent(Button, Modificators);
	}
	static void MouseUpEvent(MouseEventDispatcher::MouseButton Button, MouseEventDispatcher::ModsStatus Modificators) {
		if (!_MouseDispatcher)
			return;
		_MouseDispatcher->MouseUpEvent(Button, Modificators);
	}

	static void SetMouseDispatcher(MouseEventDispatcher *dispatcher) {
		_MouseDispatcher = dispatcher;
		AddLog(Debug, "Changed mouse dispatcher");
	}
	static void RevokeMouseDispatcher(MouseEventDispatcher *dispatcher) {
		if (_MouseDispatcher != dispatcher)
			return;
		_MouseDispatcher = nullptr;
		AddLog(Debug, "Revoked mouse dispatcher");
	}

	KeyMap& GetKeyMap() { return m_KeyMap; }

	void SetMouseDelta(const math::vec2 &MouseDelta) { m_MouseDelta = MouseDelta; }
	void ClearMouseDelta() { m_MouseDelta = math::vec2(0, 0); }

	void RefreshKeyMap();

	DefineREADAcces(MouseDelta, math::vec2);
	DefineREADAcces(KeyStatus, uint64_t);
private:
	uint64_t m_KeyStatus;
	math::fvec2 m_MouseDelta;
	KeyMap m_KeyMap;

	static MouseEventDispatcher *_MouseDispatcher;
};

} //namespace Core 

#endif

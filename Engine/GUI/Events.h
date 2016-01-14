#pragma once

namespace MoonGlare {
namespace GUI {
namespace Events {

enum class EventType {
	Unknown,

//Mouse
	MouseEnter,
	MouseLeave,

	MouseMove,

	MouseDown,
	MouseUp,

//max event value
	MaxValue,
};

struct Event {
	//Event() = delete;
	//virtual ~Event() { }
	EventType Type;

	//EventType GetType() const { return m_Type; }
protected:
	//Event(EventType type) : m_Type(type) { }
private:
};

//-----------------------------------------------------------------------------------------------
#define EVENT_CONSTRUCTOR(NAME, BASE) public: static EventType StaticType() { return EventType::NAME; }
//public: NAME##Event() : BASE(EventType::NAME) { }

struct MouseEvent : public Event {
public:
	Point MousePosition;

protected:
	//MouseEvent(EventType type) : Event(type) { }
};

struct MouseEnterEvent : public MouseEvent {
	EVENT_CONSTRUCTOR(MouseEnter, MouseEvent);
public:
};

struct MouseLeaveEvent : public MouseEvent {
	EVENT_CONSTRUCTOR(MouseLeave, MouseEvent);
public:
};

struct MouseMoveEvent : public MouseEvent {
	EVENT_CONSTRUCTOR(MouseMove, MouseEvent);
public:
	Point MouseDelta;
};

struct MouseDownEvent : public MouseEvent {
	EVENT_CONSTRUCTOR(MouseDown, MouseEvent);
public:
	MouseButton Button;
	ModsStatus Mods;
};

struct MouseUpEvent : public MouseEvent {
	EVENT_CONSTRUCTOR(MouseUp, MouseEvent);
public:
	MouseButton Button;
	ModsStatus Mods;
};

#undef EVENT_CONSTRUCTOR

} // namespace Events
} // namespace GUI 
} // namespace MoonGlare 

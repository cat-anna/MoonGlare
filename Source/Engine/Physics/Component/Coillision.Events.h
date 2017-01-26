/*
* Generated by cppsrc.sh
* by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

namespace MoonGlare::Physics::Component {	  

template<typename OWNER, Entity OWNER::*ptr>
inline Entity ExtractEntity(const OWNER *owner) {
	return (owner->*ptr);
}

struct OnCollisionEnterEvent {
	static constexpr char* EventName = "OnCollisionEnterEvent";
	static constexpr char* HandlerName = "CollisionEnter";

	Entity m_Source;
	Entity m_Object;

	btVector3 m_Normal;

	friend std::ostream& operator<<(std::ostream& out, const OnCollisionEnterEvent &event) {
		out << "OnCollisionEnterEvent"
			<< "[Source:" << event.m_Source
			<< " Object:" << event.m_Object
			<< " Normal:" << event.m_Normal
			<< "]";
		return out;
	}

	static ApiInitializer RegisterLuaApi(ApiInitializer api) {
		return api
			.beginClass<OnCollisionEnterEvent>("cOnCollisionEnterEvent")
				.addData("Source", &OnCollisionEnterEvent::m_Source, false)
				.addData("Object", &OnCollisionEnterEvent::m_Object, false)
			.endClass();
	}
};

struct OnCollisionLeaveEvent {
	static constexpr char* EventName = "OnCollisionLeaveEvent";
	static constexpr char* HandlerName = "CollisionLeave";

	Entity m_Source;
	Entity m_Object;

	btVector3 m_Normal;

	friend std::ostream& operator<<(std::ostream& out, const OnCollisionLeaveEvent &event) {
		out << "OnCollisionLeaveEvent"
			<< "[Source:" << event.m_Source
			<< " Object:" << event.m_Object
			<< " Normal:" << event.m_Normal
			<< "]";
		return out;
	}

	static ApiInitializer RegisterLuaApi(ApiInitializer api) {
		return api
			.beginClass<OnCollisionLeaveEvent>("cOnCollisionLeaveEvent")
				.addData("Source", &OnCollisionLeaveEvent::m_Source, false)
				.addData("Object", &OnCollisionLeaveEvent::m_Object, false)
			.endClass();
	}
};

} //namespace MoonGlare::Physics::Component
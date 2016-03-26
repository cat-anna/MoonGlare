/*
 * cPathControler.cpp
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "PathController.h"

namespace MoveControllers {

GABI_IMPLEMENT_CLASS(PathController)
IMPLEMENT_SCRIPT_EVENT_VECTOR(PathControllerScriptEvents);
RegisterApiDerivedClass(PathController, &PathController::RegisterScriptApi);
MoveControllerClassRegister::Register<PathController> PathControllerReg;

PathController::PathController(::Core::Objects::Object *Owner):
		BaseClass(Owner), 
		m_PositionPathState(), 
		m_LookAtPathState(), 
		m_PositionPath(), 
		m_LookAtPath(), 
		m_PositionTimer() {
	SetScriptHandlers(new PathControllerScriptEvents());
}

PathController::~PathController() {
}

PathController::PathController(const PathController& source, ::Core::Objects::Object *Owner):
		BaseClass(source, Owner),
		m_PositionPathState(source.m_PositionPathState), 
		m_LookAtPathState(source.m_LookAtPathState), 
		m_PositionPath(source.m_PositionPath), 
		m_LookAtPath(source.m_LookAtPath), 
		m_PositionTimer(source.m_PositionTimer) {
	SetScriptHandlers(new PathControllerScriptEvents());
	LOG_NOT_IMPLEMENTED();
}

iMoveController* PathController::Duplicate(::Core::Objects::Object* NewOwner) const {
	return new ThisClass(*this, NewOwner);
}

template<class R, class I>
R UnionCast(I i) {
	union {
		I _i;
		R _r;
	};
	_i = i;
	return _r;
}

using DataClasses::Paths::PathCalcState;
template<PathCalcState PathController::*PTR, float PathCalcState::*VALUE>
struct T {
	using ptr_t = float PathController::*;
	static ptr_t cast() {
		return UnionCast<ptr_t>(UnionCast<unsigned long>(PTR) + UnionCast<unsigned long>(VALUE));
	}
};

void PathController::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cPathMoveController")
		.addFunction("InvokeOnPositionReached", &ThisClass::InvokeOnPositionReached)
		.addFunction("InvokeOnLoop", &ThisClass::InvokeOnLoop)

		.addProperty("Repetitive", &ThisClass::IsRepetitive, &ThisClass::SetRepetitive)
		.addData("Speed", T<&ThisClass::m_PositionPathState, &PathCalcState::Speed>::cast())
		.addData("Position", T<&ThisClass::m_PositionPathState, &PathCalcState::Position>::cast())
		//.addProperty("LookSpeed", &ThisClass::GetLookSpeed, &ThisClass::SetLookSpeed)
		//.addProperty("LookPosition", &ThisClass::GetLookPosition, &ThisClass::SetLookPosition)
	.endClass()
	;
}

//-------------------------------------------------------------------------------------------------

int PathController::InvokeOnPositionReached(float position, int param) { SCRIPT_INVOKE(OnPositionReached, m_Owner, position, param); }
int PathController::InvokeOnStart() { SCRIPT_INVOKE(OnStart, m_Owner); }
int PathController::InvokeOnStop() { SCRIPT_INVOKE(OnStop, m_Owner); }
int PathController::InvokeOnLoop() { SCRIPT_INVOKE(OnLoop, m_Owner); }

//-------------------------------------------------------------------------------------------------

void PathController::DoMove(const ::Core::MoveConfig& conf){
	if (!IsRunning()) return;

	if (!m_PositionTimer.empty()) {
		if (m_PositionPathState.Speed >= 0) {
			if (m_PositionTimer.front().first <= m_PositionPathState.Position) {
				auto timer = m_PositionTimer.front();
				m_PositionTimer.pop_front();
				m_PositionTimer.push_back(timer);
				InvokeOnPositionReached(m_PositionPathState.Position, timer.second);
			}
		} else {
			if (m_PositionTimer.back().first >= m_PositionPathState.Position) {
				auto timer = m_PositionTimer.back();
				m_PositionTimer.pop_back();
				m_PositionTimer.push_front(timer);
				InvokeOnPositionReached(m_PositionPathState.Position, timer.second);
			}
		}
	}

	btQuaternion q;

	m_PositionPath->UpdatePosition(m_PositionPathState);
	auto point3 = m_PositionPathState.Location + m_PositionPathState.Direction;
	if (m_LookAtPath) {
		m_LookAtPath->UpdatePosition(m_LookAtPathState);
		
		auto Pnormal = math::PlaneNormalPointPointVector(
				m_LookAtPathState.Location, 
				m_PositionPathState.Location, 
				m_PositionPathState.Direction);

		auto len = Pnormal.length();
		float Angle;
		if (len < math::Constants::epsilon) {
			auto cross = m_PositionPathState.Direction.cross(Physics::vec3(0, 1, 0));

			Pnormal = math::PlaneNormalPointPointVector(
				m_LookAtPathState.Location,
				m_PositionPathState.Location,
				cross);

			//Pnormal = Physics::vec3(0, 1, 0);// m_PositionPathState.Direction.cross(Physics::vec3(1, 0, 0));

			Angle = math::Constants::pi::value;
		} else {
			auto LookDirection = m_PositionPathState.Location - m_LookAtPathState.Location;
			LookDirection.normalize();
			auto MoveDirection = m_PositionPathState.Direction;
			float cosval = LookDirection.dot(MoveDirection) / (LookDirection.length() * MoveDirection.length());
			float acval = acosf(cosval);
			Angle = acval;
			//AddLog(Debug, "ncosval:" << cosval);
		}// math::Constants::pi::half + math::Constants::pi::value math::Constants::pi::half
		//auto delta = (Look.Position - Pos.Position).normalize();
		Pnormal.normalize();
		q = btQuaternion(Pnormal, Angle);
		//AddLog(Debug, "n:" << Pnormal << "   a:" << Angle);
	} else {
		LOG_NOT_IMPLEMENTED();
	}

	//AddLog(Hint, convert(delta) << "    " << convert(rot.getAxis()) << "    " << rot.getAngle());

	auto currpos = m_Owner->GetPosition();

	m_Owner->SetPosition(m_PositionPathState.Location, q);
	m_Owner->UpdateMotionState();

	if (m_PositionPathState.Position <= 0.0f || m_PositionPathState.Position >= 1.0f) {
		if (!IsRepetitive())
			Stop();
		else
			InvokeOnLoop();
	}
}

//-------------------------------------------------------------------------------------------------

bool PathController::LoadFromXML(const xml_node Node) {
	BaseClass::LoadFromXML(Node);

	auto &map = GetOwner()->GetScene()->GetMapInstance();

	xml_node Position = Node.child("Position");
	m_PositionPath = map->GetPath(Position.attribute("Path").as_string("??"));
	m_PositionPathState.Position = Position.attribute("Position").as_float(0.0f);
	m_PositionPathState.Speed = Position.attribute("Speed").as_float(0.01f);
	SetRepetitive(m_PositionPath->IsRepetitive());

	xml_node LookAt = Node.child("LookAt");
	m_LookAtPath = map->GetPath(LookAt.attribute("Path").as_string("??"));
	m_LookAtPathState.Position = LookAt.attribute("Position").as_float(0.0f);
	m_LookAtPathState.Speed = LookAt.attribute("Speed").as_float(0.01f);

	XML::ForEachChild(Node.child("PositionTimers"), "Item", [this](xml_node node) -> int {
		PositionTimerEntry p;
		p.first = node.attribute("Position").as_float(-1);
		p.second = node.attribute("Param").as_int(0);

		if (p.first < 0.0f || p.first > 1.0f) {
			AddLog(Warning, "Invalid PathController timer position!");
			return 0;
		}
		m_PositionTimer.push_back(p);
		return 0;
	});
	m_PositionTimer.emplace_front(0.0f, 0);
	m_PositionTimer.sort();

	xml_node Flags = Node.child("Flags");
	SetRepetitive(Flags.attribute("Repetitive").as_bool(IsRepetitive()));
	return true;
}

bool operator <(const PathController::PositionTimerEntry &p1, const PathController::PositionTimerEntry &p2) { return p1.first < p2.first; }

void PathController::SetPositionTimer(float pos, int param){
	m_PositionTimer.emplace_back(pos, param);
	m_PositionTimer.sort();
}

} // namespace MoveControlers

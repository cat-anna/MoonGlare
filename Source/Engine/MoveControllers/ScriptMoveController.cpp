/*
 * cScriptMoveController.cpp
 *
 *  Created on: 28-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "ScriptMoveController.h"
namespace MoveControllers {

GABI_IMPLEMENT_CLASS(ScriptMoveController);
IMPLEMENT_SCRIPT_EVENT_VECTOR(ScriptControllerScriptEvents);
RegisterApiDerivedClass(ScriptMoveController, &ScriptMoveController::RegisterScriptApi);
//MoveControllerClassRegister::Register<PathController> PathControllerReg;

ScriptMoveController::ScriptMoveController(const ScriptMoveController& source, ::Core::Objects::Object* Owner) :
		iMoveController(source, Owner),
		m_StaticRotation(source.m_StaticRotation),
		m_StaticSpeed(source.m_StaticSpeed),
		m_ProgressGoal(source.m_ProgressGoal),
		m_Progress(source.m_Progress){
	LOG_NOT_IMPLEMENTED;
	//SetScriptHandlers(source.m_ScriptHandlers->Duplicate());
}

ScriptMoveController::ScriptMoveController(::Core::Objects::Object* Owner) :
		iMoveController(Owner),
		m_StaticRotation(),
		m_StaticSpeed(),
		m_ProgressGoal(),
		m_Progress(){
	SetScriptHandlers(new ScriptControllerScriptEvents());
}

ScriptMoveController::~ScriptMoveController() {
}

//-------------------------------------------------------------------------------------------------

void ScriptMoveController::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<MoveControllers::ScriptMoveController, iMoveController>("cScriptMover")
		.addFunction("SetStaticRotation", &MoveControllers::ScriptMoveController::SetStaticRotation)
		.addFunction("SetStaticSpeed", &MoveControllers::ScriptMoveController::SetStaticSpeed)
		.addFunction("SetDelay", &MoveControllers::ScriptMoveController::SetDelay)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

void ScriptMoveController::DoMove(const ::Core::MoveConfig& conf) {
	if (!IsRunning()) return;
	LOG_NOT_IMPLEMENTED;
#if 0

	auto Position = convert(m_Owner->GetMotionState().GetPosition());
	auto Rotation = convert(m_Owner->GetMotionState().GetRotation());
	m_Progress += conf.FrameTime;
	if (m_Progress >= m_ProgressGoal){
		m_Progress -= m_ProgressGoal;
		InvokeOnProgress();
	}
	Position += m_StaticSpeed * conf.FrameTime;
	Rotation += m_StaticRotation * conf.FrameTime;
	m_Owner->GetMotionState().SetLocation(convert(Position), convert(Rotation));

#endif // 0
}

iMoveController* ScriptMoveController::Duplicate(::Core::Objects::Object* NewOwner) const {
	return new ThisClass(*this, NewOwner);
}

//-------------------------------------------------------------------------------------------------

int ScriptMoveController::InvokeOnStart() { SCRIPT_INVOKE(OnStart, m_Owner); }
int ScriptMoveController::InvokeOnStop() { SCRIPT_INVOKE(OnStop, m_Owner); }
int ScriptMoveController::InvokeOnProgress() { SCRIPT_INVOKE(OnProgress, m_Owner); }

//-------------------------------------------------------------------------------------------------

void ScriptMoveController::SetStaticRotation(float x, float y, float z) {
	m_StaticRotation = math::vec3(x, y, z);
}

void ScriptMoveController::SetStaticSpeed(float x, float y, float z) {
	m_StaticSpeed = math::vec3(x, y, z);
}

void ScriptMoveController::SetDelay(float seconds) {
	m_ProgressGoal = seconds;
}


} // namespace MoveControllers

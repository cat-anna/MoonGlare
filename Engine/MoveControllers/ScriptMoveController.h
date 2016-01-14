/*
 * cScriptMoveController.h
 *
 *  Created on: 28-11-2013
 *      Author: Paweu
 */

#ifndef CSCRIPTMOVECONTROLLER_H_
#define CSCRIPTMOVECONTROLLER_H_

namespace MoveControllers {

DECLARE_SCRIPT_EVENT_VECTOR(ScriptControllerScriptEvents, MoveControllerScriptEvents,
	SCRIPT_EVENT_ADD((OnProgress)),
	SCRIPT_EVENT_REMOVE());

class ScriptMoveController: public iMoveController {
	GABI_DECLARE_CLASS(ScriptMoveController, iMoveController);
	DECLARE_SCRIPT_HANDLERS(ScriptControllerScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ScriptMoveController(::Core::Objects::Object *Owner = 0);
	~ScriptMoveController();

	virtual void DoMove(const ::Core::MoveConfig& conf);
	virtual iMoveController* Duplicate(::Core::Objects::Object *NewOwner) const;

//functions for scripts
	void SetStaticRotation(float x, float y, float z);
	void SetStaticSpeed(float x, float y, float z);
	void SetDelay(float seconds);

//script invokers
	virtual int InvokeOnStart();
	virtual int InvokeOnStop();
	virtual int InvokeOnProgress();
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	ScriptMoveController(const ScriptMoveController& source, ::Core::Objects::Object *Owner);
	math::vec3 m_StaticRotation, m_StaticSpeed;
	float m_ProgressGoal, m_Progress;
};

} // namespace MoveControllers
#endif // CSCRIPTMOVECONTROLLER_H_

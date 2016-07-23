/*
 * cPathControler.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef PATHCONTROLER_H_
#define PATHCONTROLER_H_

namespace MoveControllers {

DECLARE_SCRIPT_EVENT_VECTOR(PathControllerScriptEvents, MoveControllerScriptEvents,
	SCRIPT_EVENT_ADD((OnPositionReached)(OnLoop)),
	SCRIPT_EVENT_REMOVE());

class PathController : public iMoveController {
	SPACERTTI_DECLARE_CLASS(PathController, iMoveController);
	DECLARE_SCRIPT_HANDLERS(PathControllerScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	PathController(MoonGlare::Core::Objects::Object *Owner = 0);
	~PathController();

	virtual void DoMove(const MoonGlare::Core::MoveConfig& conf);
	virtual iMoveController* Duplicate(MoonGlare::Core::Objects::Object *NewOwner) const;

	virtual int InvokeOnPositionReached(float position, int param);
	virtual int InvokeOnStart();
	virtual int InvokeOnStop();
	virtual int InvokeOnLoop();

	virtual bool LoadFromXML(const xml_node Node);

	struct Flags : public BaseClass::Flags {
		enum {
			Repetitive		= 0x10,
		};
	};

	void SetPositionTimer(float pos, int param);

	DefineREADAcces(PositionPath, iPathSharedPtr);
	DefineREADAcces(LookAtPath, iPathSharedPtr);

	DefineRefGetterAll(PositionPathState, MoonGlare::DataClasses::Paths::PathCalcState);
	DefineRefGetterAll(LookAtPathState, MoonGlare::DataClasses::Paths::PathCalcState);

	DefineFlag(m_Flags, Flags::Repetitive, Repetitive);

	static void RegisterScriptApi(ApiInitializer &api);

	using PositionTimerEntry = std::pair<float, int>;
	using PositionTimer = std::list<PositionTimerEntry>;
protected:
	MoonGlare::DataClasses::Paths::PathCalcState m_PositionPathState, m_LookAtPathState;
	iPathSharedPtr m_PositionPath, m_LookAtPath;
	PositionTimer m_PositionTimer;

	PathController(const PathController& source, MoonGlare::Core::Objects::Object *Owner);
};


} // namespace MoveControlers
#endif // PATHCONTROLER_H_ 

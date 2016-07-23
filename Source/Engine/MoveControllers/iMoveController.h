/*
 * cMoveControler.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */

#ifndef CMOVECONTROLER_H_
#define CMOVECONTROLER_H_

namespace MoveControllers {

DECLARE_SCRIPT_EVENT_VECTOR(MoveControllerScriptEvents, ::Core::Scripts::iScriptEvents,
		SCRIPT_EVENT_ADD((OnStart)(OnStop)),
		SCRIPT_EVENT_REMOVE());

/**
	Interface for move controllers
	
*/
class iMoveController : public cRootClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iMoveController, cRootClass);
	DECLARE_SCRIPT_HANDLERS_ROOT(MoveControllerScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	iMoveController(MoonGlare::Core::Objects::Object *Owner = 0);
	virtual ~iMoveController();
	virtual bool SaveToXML(xml_node Node) const;
	virtual bool LoadFromXML(const xml_node Node);

	virtual bool Initialize();
	virtual bool Finalize();
	virtual void DoMove(const MoonGlare::Core::MoveConfig& conf) = 0;
	virtual iMoveController* Duplicate(MoonGlare::Core::Objects::Object *NewOwner) const = 0;

	virtual void Start();
	virtual void Stop();

	virtual int InvokeOnStart();
	virtual int InvokeOnStop();

	DefineFULLAccesPTR(Owner, MoonGlare::Core::Objects::Object);
	DefineREADAcces(Flags, unsigned);

	static iMoveController* CreateFromXML(const xml_node Node, MoonGlare::Core::Objects::Object *Owner);

	enum eMoveControllerFlags {
		mfRunning			= 0x10000000,
		mfFinished			= 0x20000000,
	};

	//iMoveController uses only the oldest byte of flags
	struct Flags {
		enum {
			Running = mfRunning,
		};
	};

	static void RegisterScriptApi(::ApiInitializer &api);

	DefineFlagGetter(m_Flags, Flags::Running, Running)
protected:
	MoonGlare::Core::Objects::Object *m_Owner;
	unsigned m_Flags;
	iMoveController(const iMoveController& other, MoonGlare::Core::Objects::Object *Owner);
	DefineFlagSetter(m_Flags, Flags::Running, Running)
};

} // namespace MoveControllers
#endif // CMOVECONTROLER_H_

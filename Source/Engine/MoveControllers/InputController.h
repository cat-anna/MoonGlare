/*
 * cInputController.h
 *
 *  Created on: 01-11-2013
 *      Author: Paweu
 */

#ifndef CINPUTCONTROLLER_H_
#define CINPUTCONTROLLER_H_

namespace MoveControllers {

DECLARE_SCRIPT_EVENT_VECTOR(InputControllerScriptEvents, MoveControllerScriptEvents,
		SCRIPT_EVENT_ADD(
			(OnKeyA)(OnKeyB)(OnKeyC)(OnKeyD)(OnKeyE)(OnKeyF)(OnKeyG)(OnKeyH)
			(OnMoveStart)(OnMoveStop)
			),
		SCRIPT_EVENT_REMOVE());

class InputController : public iMoveController {
	SPACERTTI_DECLARE_CLASS(InputController, iMoveController);
	DECLARE_SCRIPT_HANDLERS(InputControllerScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	virtual ~InputController();
	InputController(const InputController& other, MoonGlare::Core::Objects::Object *Owner);
	InputController(MoonGlare::Core::Objects::Object *Owner = 0);

	virtual bool SaveToXML(xml_node Node) const;
	virtual bool LoadFromXML(const xml_node Node);

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	virtual void DoMove(const ::Core::MoveConfig& conf) override;

	virtual void Start();
	virtual void Stop();

	virtual int InvokeOnStart() override;
	virtual int InvokeOnStop() override;
	virtual int InvokeOnKeyA();
	virtual int InvokeOnKeyB();
	virtual int InvokeOnKeyC();
	virtual int InvokeOnKeyD();
	virtual int InvokeOnKeyE();
	virtual int InvokeOnKeyF();
	virtual int InvokeOnKeyG();
	virtual int InvokeOnKeyH();
	virtual int InvokeOnMoveStart();
	virtual int InvokeOnMoveStop();

	virtual iMoveController* Duplicate(MoonGlare::Core::Objects::Object *NewOwner) const;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	vec2 m_Rotation;
	float m_SpeedModifier, m_RunSpeedModifier;
	unsigned m_UserKeysType, m_UserKeyStatus;
	bool m_MoveState;
};

} // namespace MoveControllers
#endif // CINPUTCONTROLLER_H_

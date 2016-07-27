/*
 * cInputController.cpp
 *
 *  Created on: 01-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "InputController.h"

namespace MoveControllers {

SPACERTTI_IMPLEMENT_CLASS(InputController);
IMPLEMENT_SCRIPT_EVENT_VECTOR(InputControllerScriptEvents);
RegisterApiDerivedClass(InputController, &InputController::RegisterScriptApi);
MoveControllerClassRegister::Register<InputController> InputControllerReg;

InputController::InputController(MoonGlare::Core::Objects::Object *Owner) :
		iMoveController(Owner),
		m_SpeedModifier(1.0f),
		m_RunSpeedModifier(2.0f),
		m_UserKeysType(),
		m_UserKeyStatus(),
		m_MoveState(false)
{
	SetScriptHandlers(new InputControllerScriptEvents());
#if 0
	m_Input.GetKeyMap().SetValue(87, KEYSTATUS_UP);
	m_Input.GetKeyMap().SetValue(83, KEYSTATUS_DOWN);
	m_Input.GetKeyMap().SetValue(65, KEYSTATUS_LEFT);
	m_Input.GetKeyMap().SetValue(68, KEYSTATUS_RIGHT);
	m_Input.GetKeyMap().SetValue(81, KEYSTATUS_USER_A);
	m_Input.GetKeyMap().SetValue(69, KEYSTATUS_USER_B);
	m_Input.GetKeyMap().SetValue(GLFW_KEY_LEFT_SHIFT, KEYSTATUS_RUN); //shift
#endif
}

InputController::InputController(const InputController& other, MoonGlare::Core::Objects::Object *Owner) :
		iMoveController(Owner),
		m_SpeedModifier(other.m_SpeedModifier),
		m_RunSpeedModifier(other.m_RunSpeedModifier),
		m_UserKeysType(other.m_UserKeysType),
		m_UserKeyStatus(other.m_UserKeyStatus)
{
	//SetScriptHandlers(other.m_ScriptHandlers->Duplicate());
}

InputController::~InputController() {
}

//-------------------------------------------------------------------------------------------------

bool InputController::Initialize() {
	return BaseClass::Initialize();
}

bool InputController::Finalize() {
	return BaseClass::Finalize();
}

void InputController::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<InputController, iMoveController>("cInputController")
		.addData("WalkSpeed", &ThisClass::m_SpeedModifier)
		.addData("RunSpeed", &ThisClass::m_RunSpeedModifier)

		.addFunction("InvokeOnMoveStart", &ThisClass::InvokeOnMoveStart)
		.addFunction("InvokeOnMoveStop", &ThisClass::InvokeOnMoveStop)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

const sFlagStruct InputControlerFlagPack[] = {
//		{ "FollowPath", cPathController::mfFollowPath, true },
	{ 0, 0, false }, 
};

const sFlagStruct InputControlerFlagKeyTypes[] = { 
		{ "KeyA",	KeyFlags::User_A,	false }, 
		{ "KeyB",	KeyFlags::User_B,	false }, 
		{ "KeyC",	KeyFlags::User_C,	false }, 
		{ "KeyD",	KeyFlags::User_D,	false }, 
		{ "KeyE",	KeyFlags::User_E,	false }, 
		{ "KeyF",	KeyFlags::User_F,	false }, 
		{ "KeyG",	KeyFlags::User_G,	false }, 
		{ "KeyH",	KeyFlags::User_H,	false }, 
		{ 0,		0,					false }, 
};

bool InputController::SaveToXML(xml_node Node) const {
	BaseClass::SaveToXML(Node);
	WriteFlagPack(Node.append_child("Flags"), m_Flags, InputControlerFlagPack);
	WriteFlagPack(Node.append_child("KeyTypes"), m_UserKeysType, InputControlerFlagKeyTypes);
	xml_node Speeds = Node.append_child("Speeds");
	Speeds.append_attribute("Walk") = m_SpeedModifier;
	Speeds.append_attribute("Run") = m_RunSpeedModifier;
	return true;
}

bool InputController::LoadFromXML(const xml_node Node) {
	BaseClass::LoadFromXML(Node);
	ReadFlagPack(Node.child("Flags"), m_Flags, InputControlerFlagPack);
	ReadFlagPack(Node.child("KeyTypes"), m_UserKeysType, InputControlerFlagKeyTypes);
	const xml_node Speeds = Node.child("Speeds");
	m_SpeedModifier = Speeds.attribute("Walk").as_float(m_SpeedModifier);
	m_RunSpeedModifier = Speeds.attribute("Run").as_float(m_RunSpeedModifier);
	return true;
}

iMoveController* InputController::Duplicate(MoonGlare::Core::Objects::Object *NewOwner) const {
	return new InputController(*this, NewOwner);
}

//-------------------------------------------------------------------------------------------------

void InputController::Start() {
	BaseClass::Start();
}

void InputController::Stop() {
	if (m_MoveState)
		InvokeOnMoveStop();
	m_MoveState = false;
	BaseClass::Stop();
}

//-------------------------------------------------------------------------------------------------

void InputController::DoMove(const ::Core::MoveConfig& conf) {
	if (!IsRunning()) return;

	float MouseSensitivity = ::Settings->Input.MouseSensitivity;
	MouseSensitivity *= Settings->Input.GetStaticMouseSensivity();
	auto &MouseDelta = MoonGlare::Core::GetInput()->GetMouseDelta();

	Physics::vec3 LinearSpeed(0, 0, 0);
	Physics::vec3 Rotation(0, 0, 0);
	bool NewMoveState = false;
	bool MoveAllowed = true;
	bool MoveNeeded = false;

	if (MouseDelta[0] != 0) {
		m_Rotation[0] -= MouseDelta[0] * MouseSensitivity;
		MoveNeeded = true;
	}
	if (MouseDelta[1] != 0) {
		m_Rotation[1] = math::clamp(m_Rotation[1] - MouseDelta[1] * MouseSensitivity / 2.0f, 
									-math::Constants::pi::deg_60, math::Constants::pi::deg_60);
		MoveNeeded = true;
	}

	Physics::vec3 LookDirection(
			cos(m_Rotation[1]) * sin(m_Rotation[0]),
			sin(m_Rotation[1]),
			cos(m_Rotation[1]) * cos(m_Rotation[0]));
   
	LookDirection.normalize();

	Physics::vec3 AngleDirection(sin(m_Rotation[0]), 0.0f, cos(m_Rotation[0]));

	auto keys = MoonGlare::Core::GetInput()->GetKeyStatus();
	if (keys & KeyFlags::Move_Mask) {
		NewMoveState = true;
		if (keys & (KeyFlags::Move_Up | KeyFlags::Move_Down)) {
			MoveNeeded = true;
			if (keys & KeyFlags::Move_Up) {  
				LinearSpeed = LookDirection;// AngleDirection;
			} else {  
				LinearSpeed = -LookDirection;// -AngleDirection;
			}
		}

		if (keys & (KeyFlags::Move_Left | KeyFlags::Move_Right)) {
			MoveNeeded = true;
			float roty_90 = m_Rotation[0] + math::Constants::pi::half;
			Physics::vec3 direction(sin(roty_90), 0.0f, cos(roty_90));
			//direction *= 0.8f;
			if (keys & KeyFlags::Move_Left) {  
				LinearSpeed += direction;
			} else {  
				LinearSpeed -= direction;
			}
		}

		if (keys & KeyFlags::Run)
			LinearSpeed *= (m_RunSpeedModifier);
		else
			LinearSpeed *= (m_SpeedModifier);
	}
	
	if (m_MoveState != NewMoveState) {
		int callret;
		if (NewMoveState) {
			callret = InvokeOnMoveStart();
			if (callret) {
				NewMoveState = false;
				MoveAllowed = false;
				MoveNeeded = false;
			}
		} else
			callret = InvokeOnMoveStop();

		m_MoveState = NewMoveState;
	}

	if (MoveAllowed && MoveNeeded) {
	//	btQuaternion q(Physics::vec3(0, 1, 0), m_Rotation[1]);

		m_Owner->SetLookDirection(LookDirection); 
		//auto body = m_Owner->GetBody();
		//LinearSpeed[1] = body->GetLinearVelocity()[1];
		//body->SetLinearVelocity(LinearSpeed);
		//m_Owner->GetBody()->ApplyCentralForce(LinearSpeed * 700.0f);
		//motion.SetQuaterion(q);
		//motion.Update();

		m_Owner->SetPosition(m_Owner->GetPosition() + LinearSpeed * conf.TimeDelta);
		m_Owner->UpdateMotionState();
	}

	if (keys & KeyFlags::User_Mask) {
		unsigned KeyStatus = keys & (m_UserKeysType | ~m_UserKeyStatus);
		m_UserKeyStatus = ~m_UserKeysType & keys;
#define CheckKey(NAME)   if(KeyStatus & KeyFlags::User_##NAME) InvokeOnKey##NAME(); 
		CheckKey(A)
		CheckKey(B)
		CheckKey(C)
		CheckKey(D)
		CheckKey(E)
		CheckKey(F)
		CheckKey(G)
		CheckKey(H)
#undef CheckKey
	}
	else
		m_UserKeyStatus = 0;
}

//-------------------------------------------------------------------------------------------------

int InputController::InvokeOnStart() { SCRIPT_INVOKE(OnStart, m_Owner); }
int InputController::InvokeOnStop() { SCRIPT_INVOKE(OnStop, m_Owner); }
int InputController::InvokeOnKeyA() { SCRIPT_INVOKE(OnKeyA, m_Owner); }
int InputController::InvokeOnKeyB() { SCRIPT_INVOKE(OnKeyB, m_Owner); }
int InputController::InvokeOnKeyC() { SCRIPT_INVOKE(OnKeyC, m_Owner); }
int InputController::InvokeOnKeyD() { SCRIPT_INVOKE(OnKeyD, m_Owner); }
int InputController::InvokeOnKeyE() { SCRIPT_INVOKE(OnKeyE, m_Owner); }
int InputController::InvokeOnKeyF() { SCRIPT_INVOKE(OnKeyF, m_Owner); }
int InputController::InvokeOnKeyG() { SCRIPT_INVOKE(OnKeyG, m_Owner); }
int InputController::InvokeOnKeyH() { SCRIPT_INVOKE(OnKeyH, m_Owner); }
int InputController::InvokeOnMoveStart() { SCRIPT_INVOKE(OnMoveStart, m_Owner); }
int InputController::InvokeOnMoveStop() { SCRIPT_INVOKE(OnMoveStop, m_Owner); }

} // namespace MoveControllers

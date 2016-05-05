/*
 * cMoveControler.cpp
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */
#include <pch.h>
//#include "MoveContollers.h"
#include <MoonGlare.h>

namespace MoveControllers {

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iMoveController);
IMPLEMENT_SCRIPT_EVENT_VECTOR(MoveControllerScriptEvents);
RegisterApiDerivedClass(iMoveController, &iMoveController::RegisterScriptApi);

iMoveController::iMoveController(::Core::Objects::Object *Owner) :
		BaseClass(), 
		m_Owner(Owner), 
		m_Flags(),
		m_ScriptHandlers() 
{
	SetRunning(true);
}

iMoveController::iMoveController(const iMoveController& other, ::Core::Objects::Object *Owner) :
		BaseClass(),
		m_Owner(Owner), 
		m_Flags(other.m_Flags),
		m_ScriptHandlers()
{
	LOG_NOT_IMPLEMENTED();
}

iMoveController::~iMoveController() {
}

//-------------------------------------------------------------------------------------------------

bool iMoveController::Initialize() {
	if (IsRunning())
		Start();
	return true;
}

bool iMoveController::Finalize() {
	if (IsRunning())
		Stop();
	return true;
}

void iMoveController::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("iMoveController")
		.addFunction("InvokeOnStart", &ThisClass::InvokeOnStart)
		.addFunction("InvokeOnStop", &ThisClass::InvokeOnStop)
		.addFunction("SetEventFunction", &ThisClass::SetEventFunction)
		.addFunction("Stop", &ThisClass::Stop)
		.addFunction("Start", &ThisClass::Start)
		.addFunction("Duplicate", &ThisClass::Duplicate)
		.addProperty("Running", &ThisClass::IsRunning)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

bool iMoveController::SaveToXML(xml_node Node) const {
	Node.append_attribute("Class") = GetDynamicTypeInfo()->Name;
	//m_ScriptHandlers->SaveToXML(Node);
	return true;
}

bool iMoveController::LoadFromXML(const xml_node Node) {
	m_ScriptHandlers->LoadFromXML(Node);
	xml_node Flags = Node.child("Flags");
	SetRunning(Flags.attribute("Running").as_bool(IsRunning()));
	return true;
}

//-------------------------------------------------------------------------------------------------

int iMoveController::InvokeOnStart() { SCRIPT_INVOKE(OnStart, m_Owner); }
int iMoveController::InvokeOnStop() { SCRIPT_INVOKE(OnStop, m_Owner); }

//-------------------------------------------------------------------------------------------------

void iMoveController::Start() {
	SetRunning(true);
	InvokeOnStart();
}

void iMoveController::Stop() {
	SetRunning(false);
	InvokeOnStop();
}
//-------------------------------------------------------------------------------------------------

iMoveController* iMoveController::CreateFromXML(const xml_node Node, ::Core::Objects::Object *Owner) {
	if (!Owner) {
		AddLog(Error, "Attempt to create MoveController without owner!");
		return 0;
	}
	if(!Node) return 0;
	string Class = Node.attribute("Class").as_string("");
	if (Class.empty()) {
		AddLog(Error, "MoveController definition without a class name!");
		return 0;
	}
	iMoveController *obj = MoveControllerClassRegister::CreateClass(Class);
	if (!obj) {
		AddLog(Error, "Unable to create MoveController[class:" << Class << "]");
		return 0;
	}
	obj->SetOwner(Owner);
	if(!obj->LoadFromXML(Node))
		AddLogf(Error, "Unable to load MoveController[class: %s] from XML for object[Name: %s]", Class.c_str(), Owner->GetName().c_str());
	return obj;
}

} // namespace MoveControllers 

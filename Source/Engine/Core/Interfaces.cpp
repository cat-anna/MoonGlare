/*
 * cInterfacesList.cpp
 *
 *  Created on: 03-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/DataClasses/iFont.h>

namespace MoonGlare {
namespace Core {

Interfaces *Interfaces::Instance = 0;
RegisterApiNonClass(Interfaces, &Interfaces::RegisterScriptApi);

Interfaces::Interfaces() {
}

void Interfaces::RegisterScriptApi(ApiInitializer &api) {
	api
	.beginNamespace("Interface")
		.addFunction("CreateMoveController", &MoveControllers::MoveControllerClassRegister::CreateClass)
	.endNamespace()
	;
}

void Interfaces::Initialize() {
	if (Instance) return;
	Instance = new Interfaces();
}

void Interfaces::DumpLists(ostream &out) {
	auto regdump = [&out](const Space::TemplateClassListBase::ClassInfoBase &type) {
		char buffer[256];
		sprintf(buffer, "%30s  [class %s]\n", type.Alias.c_str(), (type.TypeInfo ? type.TypeInfo->GetFullName() : "?"));
		out << buffer;
	};
	out << "\nModel class list\n";
	DataClasses::Models::ModelClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nFont class list\n";
	DataClasses::Fonts::FontClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nMap class list\n";
	DataClasses::Maps::MapClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nScene class list\n";
	Core::Scene::SceneClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nMove controllers class list\n";
	MoveControllers::MoveControllerClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nShaders class list\n";
	Graphic::Shaders::ShaderClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nLightSoure class list\n";
	Core::Objects::LightSoureClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nBody shapes class list\n";
	Physics::BodyShapeClassRegister::GetRegister()->Enumerate(regdump);	
	out << "\nGUI widgets class list\n";
	GUI::WidgetClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nSound types list\n";
	Sound::SoundClassRegister::GetRegister()->Enumerate(regdump);
	out << "\n"; 
	out << "\n";
	out << "\n";
	auto func = [&out](const Space::ClassListRegisterBase<> *r) {
		out << r->GetCompilerName() << "\n";
	};
	Space::ClassListRegisterBase<>::EnumerateRegisters(func);

	out << "\n";
	out << "\n";
	out << "\n";
}

} // namespace Core 
} //namespace MoonGlare 

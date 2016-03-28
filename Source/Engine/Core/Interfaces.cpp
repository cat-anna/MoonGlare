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

namespace Core {

struct ClassRegistersRequest {
	const char *Name;
	const GabiLib::GabiTypeInfo *Class;
	const GabiLib::GabiTypeInfo *Requirement;
	Interfaces::ClassLists List;

	static std::vector<ClassRegistersRequest> *_Requests;
};
std::vector<ClassRegistersRequest>* ClassRegistersRequest::_Requests = 0;

Interfaces *Interfaces::Instance = 0;
RegisterApiNonClass(Interfaces, &Interfaces::RegisterScriptApi);

void Interfaces::DumpList(ClassList &list, ostream &out){
	char buffer[256];
	for (auto &it : list) {
		sprintf(buffer, "%30s  [class %s]\n", it.first.c_str(), it.second.Class->Name);
		out << buffer;
	}	
} 

Interfaces::Interfaces() : m_ClassLists((unsigned)ClassLists::MaxValue){
	m_ClassLists.Name((unsigned)ClassLists::Core) = "Core";
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

	if (ClassRegistersRequest::_Requests) {
		for (auto &it : *ClassRegistersRequest::_Requests) {
			RegisterClass(it.Name, it.Class, it.Requirement, it.List);
		}
		ClassRegistersRequest::_Requests->clear();
		delete ClassRegistersRequest::_Requests;
		ClassRegistersRequest::_Requests = 0;
	}
}

void Interfaces::DumpLists(ostream &out) {
	for (auto & i : Instance->m_ClassLists){
		out  << i.Name << " class list:\n";
		DumpList(i.List, out);
		out << "\n";
	}

	auto regdump = [&out](const GabiLib::TemplateClassListBase::ClassInfoBase &type) {
		char buffer[256];
		sprintf(buffer, "%30s  [class %s]\n", type.Alias.c_str(), type.TypeInfo->GetFullName());
		out << buffer;
	};
	out << "\nModel class list\n";
	DataClasses::Models::ModelClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nFont class list\n";
	DataClasses::Fonts::FontClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nMap class list\n";
	DataClasses::Maps::MapClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nObject class list\n";
	::Core::Objects::ObjectClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nScene class list\n";
	::Core::Scene::SceneClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nMove controllers class list\n";
	MoveControllers::MoveControllerClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nShaders class list\n";
	Graphic::Shaders::ShaderClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nLightSoure class list\n";
	::Core::Objects::LightSoureClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nBody shapes class list\n";
	Physics::BodyShapeClassRegister::GetRegister()->Enumerate(regdump);	
	out << "\nGUI widgets class list\n";
	GUI::WidgetClassRegister::GetRegister()->Enumerate(regdump);
	out << "\nSound types list\n";
	Sound::SoundClassRegister::GetRegister()->Enumerate(regdump);
	out << "\n"; 
	out << "\n";
	out << "\n";
	auto func = [&out](const GabiLib::ClassListRegisterBase<> *r) {
		out << r->GetCompilerName() << "\n";
	};
	GabiLib::ClassListRegisterBase<>::EnumerateRegisters(func);

	out << "\n";
	out << "\n";
	out << "\n";
}

void Interfaces::AutoRegisterClass(const char *Name, const GabiLib::GabiTypeInfo *ClassInfo, 
								   const GabiLib::GabiTypeInfo *Requirement, ClassLists List) {
	if (!ClassRegistersRequest::_Requests)
		ClassRegistersRequest::_Requests = new std::vector<ClassRegistersRequest>();
	ClassRegistersRequest req;
	req.List = List;
	req.Class = ClassInfo;
	req.Requirement = Requirement;
	req.Name = Name;
	ClassRegistersRequest::_Requests->push_back(req);
}

void Interfaces::RegisterClass(const char*Name, const GabiLib::GabiTypeInfo *ClassInfo, const GabiLib::GabiTypeInfo *Requirement, ClassLists List) {
	if(!Instance) Instance = new Interfaces();
	if (!GabiLib::GabiObject::IsBaseClass(Requirement, ClassInfo)) {
		AddLog(Error, "Attempt to register class " << ClassInfo->Name << " to list which require " << Requirement->Name << " as a base class!");
	}
	Instance->m_ClassLists[(int)List].Add(Name, ClassInfo);
}

cRootClass* Interfaces::CreateClass(const string& ClassName, ClassLists List, const GabiLib::GabiTypeInfo *Requirement) {
	THROW_ASSERTs(Instance, 0);
	const GabiLib::GabiTypeInfo *type = Instance->m_ClassLists[(int)List].GetClass(ClassName);
	if(!type) {
		AddLogf(Error, "Class %s not found in list '%s'", ClassName.c_str(), Instance->m_ClassLists.Name((unsigned)List).c_str());
	}
	if(!GabiObject::IsBaseClass(Requirement, type)) {
		AddLogf(Error, "Requested class[Name:%s] does not inherit from required base[Name:%s]!", ClassName.c_str(), Requirement->Name);
		return 0;
	} else
		return (cRootClass*)type->NewObject();
}

//----------------------------------------------------------------------------------

void Interfaces::EnumerateClassLists(std::function<void(ClassLists listID, const string& ListName)> func) {
	for (unsigned i = 0; i < (unsigned)ClassLists::MaxValue; ++i)
		func((ClassLists)i, Instance->m_ClassLists[i].Name);
}

void Interfaces::EnumerateClassListContent(ClassLists listID, std::function<void(const ClassInfo& info)> func) {
	if (listID >= ClassLists::MaxValue) return;
	auto &list = Instance->m_ClassLists[(unsigned)listID].List;
	for (auto &it : list)
		func(it.second);
}

} // namespace Core 

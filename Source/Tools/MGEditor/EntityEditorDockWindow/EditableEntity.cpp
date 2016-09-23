/*
  * Generated by cppsrc.sh
  * On 2016-09-10 16:34:37,71
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>

#include <Source/Engine/Renderer/Light.h>

#include <Engine/GUI/Types.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <LightComponent.x2c.h>
#include <BodyComponent.x2c.h>
#include <TransformComponent.x2c.h>
#include <CameraComponent.x2c.h>
#include <MeshComponent.x2c.h>
#include <ScriptComponent.x2c.h>
#include <RectTransformComponent.x2c.h>
#include <ImageComponent.x2c.h>
#include <PanelComponent.x2c.h>

#include "EditableEntity.h"
#include <MainWindow.h>
#include <FileSystem.h>

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

//----------------------------------------------------------------------------------

namespace x2cTypes {
	using namespace x2c::Component::BodyComponent;
	using namespace x2c::Component::LightComponent;
	using namespace x2c::Component::TransfromComponent;
	using namespace x2c::Component::CameraComponent;
	using namespace x2c::Component::MeshComponent;
	using namespace x2c::Component::ScriptComponent;
	using namespace x2c::Component::RectTransformComponent;
	using namespace x2c::Component::ImageComponent;
	using namespace x2c::Component::PanelComponent;
}

template<class X2CSTRUCT> UniqueEditableComponent CreateFunction(EditableEntity *Parent, const ComponentInfo *cInfo) {
	return UniqueEditableComponent(new X2CEditableStructure<X2CSTRUCT>(Parent, cInfo));
}
using Core::Component::ComponentID;
using Core::Component::ComponentIDs;

template<typename X2CSTRUC, typename ...ARGS>
std::pair<ComponentID, ComponentInfo> MakeComponentInfo(ComponentIDs cid, ARGS&& ...args) {
	return std::make_pair((ComponentID)cid, ComponentInfo{ (ComponentID)cid, &CreateFunction<X2CSTRUC>, std::forward<ARGS>(args)..., });
}

const std::unordered_map<MoonGlare::Core::ComponentID, ComponentInfo> ComponentInfoMap {
	MakeComponentInfo<x2cTypes::TransfromEntry_t		>(ComponentIDs::Transform,		"Transform"),
	MakeComponentInfo<x2cTypes::LightEntry_t			>(ComponentIDs::Light,			"Light"),
	MakeComponentInfo<x2cTypes::BodyEntry_t				>(ComponentIDs::Body,			"Body"),
	MakeComponentInfo<x2cTypes::CameraEntry_t			>(ComponentIDs::Camera,			"Camera"),
	MakeComponentInfo<x2cTypes::MeshEntry_t				>(ComponentIDs::Mesh,			"Mesh"),
	MakeComponentInfo<x2cTypes::ScriptEntry_t			>(ComponentIDs::Script,			"Script"),
	MakeComponentInfo<x2cTypes::RectTransformEntry_t	>(ComponentIDs::RectTransform,	"RectTransform"),
	MakeComponentInfo<x2cTypes::ImageEntry_t			>(ComponentIDs::Image,			"Image"),
	MakeComponentInfo<x2cTypes::PanelEntry_t			>(ComponentIDs::Panel,			"Panel"),
};

//----------------------------------------------------------------------------------

template<typename X2CENUM>
std::pair<std::string, std::shared_ptr<TypeInfo>> MakeEnum() {
	return std::make_pair(X2CENUM::GetTypeName(), std::make_shared<TemplateTypeInfo<X2CEnumEditor<X2CENUM>>>());
}

struct bool_TypeInfo {
	static constexpr char *GetTypeName() {
		return "bool";
	}
	static bool GetValues(std::unordered_map<std::string, uint64_t> &values) {
		values["Enabled"] = static_cast<uint64_t>(1);
		values["Disabled"] = static_cast<uint64_t>(0);
		return true;
	}
};

const std::unordered_map<std::string, std::shared_ptr<TypeInfo>> TypeInfoMap{
	MakeEnum<x2cTypes::AlignMode_TypeInfo>(),
	MakeEnum<x2cTypes::ImageScaleMode_TypeInfo>(),
	MakeEnum<bool_TypeInfo>(),
};

//----------------------------------------------------------------------------------

EditableEntity::EditableEntity(EditableEntity *Parent)
	: m_Parent(Parent) {
}

EditableEntity::~EditableEntity() {
}

bool EditableEntity::Read(pugi::xml_node node) {
	m_Name = node.attribute("Name").as_string("");
	for (auto it = node.first_child(); it; it = it.next_sibling()) {
		const char *nodename = it.name();
		auto hash = Space::Utils::MakeHash32(nodename);

		switch (hash) {

		case "Component"_Hash32:
		{
			auto child = EditableComponent::CreateComponent(this, it);
			if (!child) {
				//TODO: log sth
				continue;
			}
			m_Components.emplace_back(std::move(child));
			break;
		}

		//case "Entity"_Hash32:
		//{
			//auto pattern = it.attribute("Pattern").as_string(nullptr);
			//if (pattern) {
			//	FileSystem::XMLFile xdoc;
			//	Entity child;
			//	std::string paturi = pattern;
			//	if (!GetFileSystem()->OpenXML(xdoc, paturi, DataPath::URI)) {
			//		AddLogf(Error, "Failed to open pattern: %s", pattern);
			//		continue;
			//	}
			//
			//	auto c = BuildChild(Owner, xdoc->document_element(), child);
			//	if (c == 0) {
			//		AddLogf(Error, "Failed to load child!");
			//		continue;
			//	}
			//	count += c;
			//	continue;
			//}
		//}
		//no break;
		//[[fallthrough]]
		case "Entity"_Hash32:
		case "Child"_Hash32:
		{
			UniqueEditableEntity child (new EditableChild(this));
			if (!child->Read(it)) {
				//TODO: log sth
				continue;
			}
			m_Children.emplace_back(std::move(child));
		}
		default:
			AddLogf(Warning, "Unknown node: %s", nodename);
			continue;
		}
	}

	return true;
}

bool EditableEntity::Write(pugi::xml_node node) {
	auto selfnode = node.append_child("Entity");
	selfnode.append_attribute("Name") = m_Name.c_str();

	bool ret = true;
	for (auto &it : m_Components) {
		ret = ret && it->Write(selfnode);
	}
	for (auto &it : m_Children) {
		ret = ret && it->Write(selfnode);
	}
	return ret;
}

void EditableEntity::MoveDown(EditableComponent *c) {
	auto idx = Find(c);
	if (idx < 0)
		return;
	if (idx + 1 >= m_Components.size())
		return;
	m_Components[idx].swap(m_Components[idx + 1]);
}

void EditableEntity::MoveUp(EditableComponent *c) {
	auto idx = Find(c);
	if (idx <= 0)
		return;
	m_Components[idx].swap(m_Components[idx - 1]);
}

void EditableEntity::MoveDown(EditableEntity * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;
	if (idx + 1 >= m_Children.size())
		return;
	m_Children[idx].swap(m_Children[idx + 1]);
}

void EditableEntity::MoveUp(EditableEntity * c) {
	auto idx = Find(c);
	if (idx <= 0)
		return;
	m_Children[idx].swap(m_Children[idx - 1]);
}

EditableEntity *EditableEntity::AddChild() {
	UniqueEditableEntity child(new EditableChild(this));
	child->GetName() = "child";
	auto rawptr = child.get();
	m_Children.emplace_back(std::move(child));
	return rawptr;
}

void EditableEntity::DeleteChild(EditableEntity * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;

	m_Children[idx].reset();
	for (; idx < (int)m_Children.size()-1; ++idx)
		m_Children[idx].swap(m_Children[idx + 1]);;

	m_Children.pop_back();
}

EditableComponent* EditableEntity::AddComponent(ComponentID cid) {
	auto child = EditableComponent::CreateComponent(this, cid);
	if (!child) {
		//TODO: log sth
		return nullptr;
	}
	auto rawptr = child.get();
	m_Components.emplace_back(std::move(child));
	return rawptr;
}

void EditableEntity::DeleteComponent(EditableComponent * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;

	m_Components[idx].reset();
	for (; idx < (int)m_Components.size() - 1; ++idx)
		m_Components[idx].swap(m_Components[idx + 1]);;
	m_Components.pop_back();
}

//----------------------------------------------------------------------------------

EditablePattern::EditablePattern()
	: EditableEntity(nullptr) {
}

bool EditablePattern::OpenPattern(const std::string &filename) {
	pugi::xml_document xdoc;

	auto fs = MainWindow::Get()->GetFilesystem();
	StarVFS::ByteTable bt;
	if (!fs->GetFileData(filename, bt)) {
		//todo: log sth
		return false;
	}
	if (bt.byte_size() == 0) {
		GetName() = "Empty";
		return true;
	}

	if (!xdoc.load_string((char*)bt.get())) {
		//TODO: log sth
		return false;
	}

	return Read(xdoc.document_element());
}

bool EditablePattern::SavePattern(const std::string & filename) {
	pugi::xml_document xdoc;

	if (!Write(xdoc)) {
		//TODO: log sth
		return false;
	}

	std::stringstream ss;
	xdoc.save(ss);
	StarVFS::ByteTable bt;
	bt.from_string(ss.str());

	auto fs = MainWindow::Get()->GetFilesystem();
	if (!fs->SetFileData(filename, bt)) {
		//todo: log sth
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------

EditableChild::EditableChild(EditableEntity *Parent)
	: EditableEntity(Parent) {

}

//----------------------------------------------------------------------------------

EditableComponent::EditableComponent(EditableEntity *Parent, const ComponentInfo *cInfo)
		: m_ComponentInfo(cInfo) {
	m_Parent = Parent;
}

EditableComponent::~EditableComponent() {}

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, pugi::xml_node node) {
	MoonGlare::Core::ComponentID cid = (ComponentID)ComponentIDs::Invalid;

	auto idxml = node.attribute("Id");
	if (idxml) {
		cid = idxml.as_uint(0);
	} else {
		auto namexml = node.attribute("Name");
		if (!namexml) {
			AddLogf(Error, "Component definition without id or name!");
			return nullptr;
		}
		auto namexmlstr = namexml.as_string("");
		auto it = std::find_if(ComponentInfoMap.begin(), ComponentInfoMap.end(), [namexmlstr, &cid](const std::pair<Core::ComponentID, ComponentInfo> &item) {
			if (item.second.m_Name == namexmlstr) {
				cid = item.second.m_CID;
				return true;
			}
			return false;
		});
		if (cid == (ComponentID)ComponentIDs::Invalid) {
			AddLogf(Error, "Unknown component name: %s", namexml.as_string(""));
		}
	}

	if (cid == (ComponentID)ComponentIDs::Invalid)
		return nullptr;

	auto ret = CreateComponent(Parent, cid);
	if (!ret) {
		//already logged
		return nullptr;
	}

	if (!ret->Read(node)) {
		//TODO: log sth
		return nullptr;
	}

	return std::move(ret);
}

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, MoonGlare::Core::ComponentID cid) {
	auto it = ComponentInfoMap.find(cid);
	if (it == ComponentInfoMap.end()) {
		AddLogf(Error, "Unknown component id: %d", (int)cid);
		return nullptr;
	}

	auto ret = it->second.m_CreteFunc(Parent, &it->second);
	return std::move(ret);
}

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 

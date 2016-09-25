#include PCH_HEADER

#include <Source/Engine/Core/Component/nfComponent.h>

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
#include <TextComponent.x2c.h>

#include "CustomType.h"
#include "Structure.h"
#include "ComponentInfo.h"

namespace MoonGlare {
namespace TypeEditor {

using namespace x2c::Component;// ::BodyComponent;
//using namespace x2c::Component::LightComponent;
//using namespace x2c::Component::TransfromComponent;
//using namespace x2c::Component::CameraComponent;
//using namespace x2c::Component::MeshComponent;
//using namespace x2c::Component::ScriptComponent;
//using namespace x2c::Component::RectTransformComponent;
//using namespace x2c::Component::ImageComponent;
//using namespace x2c::Component::PanelComponent;
//using namespace x2c::Component::TextComponent;

using Core::Component::ComponentID;
using Core::Component::ComponentIDs;

//
//const std::unordered_map<MoonGlare::Core::ComponentID, ComponentInfo> ComponentInfoMap{

//};

//----------------------------------------------------------------------------------

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

struct EmptySettings {
	static constexpr char *GetTypeName() { return "EmptySettings"; }
	bool Read(const pugi::xml_node node, const char *name = nullptr) { return true; }
	bool Write(pugi::xml_node node, const char *name = nullptr) const { return true; }
	void ResetToDefault() { }
	bool WriteFile(const std::string& filename) const { return false; }
	bool ReadFile(const std::string& filename) { return true; }
	void GetMemberInfo(::x2c::cxxpugi::StructureMemberInfoTable &members) const { }
	void GetWriteFuncs(std::unordered_map<std::string, std::function<void(EmptySettings &self, const std::string &input)>> &funcs) const { }
	void GetReadFuncs(std::unordered_map<std::string, std::function<void(const EmptySettings &self, std::string &output)>> &funcs) { }
};

struct RextTRansformComponentDesc {
	using Entry_t = RectTransformComponent::RectTransformEntry_t;
	using Settings_t = RectTransformComponent::RectTransformSettings_t;
	static constexpr char *DisplayName = "Gui.RectTransform";
	static constexpr char *Name = "RectTransform";
	static constexpr ComponentIDs CID = ComponentIDs::RectTransform;
	static constexpr std::array<ComponentIDs, 1> Deps = { ComponentIDs::Invalid };
};
struct ImageComponentDesc {
	using Entry_t = ImageComponent::ImageEntry_t;
	using Settings_t = ImageComponent::ImageComponentSettings_t;
	static constexpr char *DisplayName = "Gui.Image";
	static constexpr char *Name = "Image";
	static constexpr ComponentIDs CID = ComponentIDs::Image;
	static constexpr std::array<ComponentIDs, 1> Deps = { ComponentIDs::RectTransform };
};
struct PanelComponentDesc {	
	using Entry_t = PanelComponent::PanelEntry_t;
	using Settings_t = PanelComponent::PanelComponentSettings_t;
	static constexpr char *DisplayName = "Gui.Panel";
	static constexpr char *Name = "Panel";
	static constexpr ComponentIDs CID = ComponentIDs::Panel;
	static constexpr std::array<ComponentIDs, 1> Deps = { ComponentIDs::RectTransform };
};
struct TextComponentDesc {
	using Entry_t = TextComponent::TextEntry_t;
	using Settings_t = TextComponent::TextComponentSettings_t;
	static constexpr char *DisplayName = "Gui.Text";
	static constexpr char *Name = "Text";
	static constexpr ComponentIDs CID = ComponentIDs::Text;
	static constexpr std::array<ComponentIDs, 1> Deps = { ComponentIDs::RectTransform };
};

struct Register {
	Register() {
		RegEnum<RectTransformComponent::AlignMode_TypeInfo>();
		RegEnum<ImageComponent::ImageScaleMode_TypeInfo>();
		RegEnum<bool_TypeInfo>();

		RegComponent<TransfromComponent::TransfromEntry_t			, EmptySettings										>("Transform"			, "Transform"		, ComponentIDs::Transform,			{ } );
		RegComponent<LightComponent::LightEntry_t					, EmptySettings										>("Light"				, "Light"			, ComponentIDs::Light,				{ } );
		RegComponent<BodyComponent::BodyEntry_t						, BodyComponent::BodyComponentSettings_t			>("Body"				, "Body"			, ComponentIDs::Body,				{ } );
		RegComponent<CameraComponent::CameraEntry_t					, EmptySettings										>("Camera"				, "Camera"			, ComponentIDs::Camera,				{ } );
		RegComponent<MeshComponent::MeshEntry_t						, EmptySettings										>("Mesh"				, "Mesh"			, ComponentIDs::Mesh,				{ } );
		RegComponent<ScriptComponent::ScriptEntry_t					, EmptySettings										>("Script"				, "Script"			, ComponentIDs::Script,				{ } );
	
		RegComponent<RextTRansformComponentDesc>();
		RegComponent<ImageComponentDesc>();
		RegComponent<PanelComponentDesc>();
		RegComponent<TextComponentDesc>();
	}

	template<typename ENUM>
	void RegEnum() {
		TypeEditorInfo::RegisterTypeEditor(std::make_shared<TemplateTypeEditorInfo<EnumTemplate<ENUM>>>(), ENUM::GetTypeName());
	}

	template<class STRUCT>
	static UniqueStructure X2CSCreateFunc(X2CEditor *Editor, Structure *Parent) {
		return std::make_unique<StructureTemplate<STRUCT>>(Editor, Parent);
	}

	template<typename STRUCT>
	SharedStructureInfo MakeStructureInfo() {
		StructureInfo si;
		si.m_DisplayName = si.m_Name = STRUCT::GetTypeName();
		si.m_CreateFunc = &X2CSCreateFunc<STRUCT>;
		return std::make_shared<StructureInfo>(std::move(si));
	}

	template<typename STRUCT>
	SharedStructureInfo RegStructure() {
		SharedStructureInfo ssi;
		Structure::RegisterStructureInfo(ssi = MakeStructureInfo<STRUCT>());
		return ssi;
	}

	template<typename STRUCT, typename SETTINGS>
	void RegComponent(const char *DisplayName, const char *Name, ComponentIDs cid, std::initializer_list<ComponentIDs> deps = {}) {
		ComponentInfo ci;
		ci.m_CID = static_cast<ComponentID>(cid);
		ci.m_Name = Name;
		ci.m_DisplayName = DisplayName;
		ci.m_EntryStructure = RegStructure<STRUCT>();
		ci.m_SettingsStructure = RegStructure<SETTINGS>();
		ci.m_Requirements.reserve(deps.size());
		for (auto it : deps)
			if (it != ComponentIDs::Invalid)
				ci.m_Requirements.push_back(static_cast<ComponentID>(it));
		ComponentInfo::RegisterComponentInfo(std::make_shared<ComponentInfo>(std::move(ci)));
	}

	template<typename CINFO>
	void RegComponent() {
		ComponentInfo ci;
		ci.m_CID = static_cast<ComponentID>(CINFO::CID);
		ci.m_Name = CINFO::Name;
		ci.m_DisplayName = CINFO::DisplayName;
		ci.m_EntryStructure = RegStructure<CINFO::Entry_t>();
		ci.m_SettingsStructure = RegStructure<CINFO::Settings_t>();
		ci.m_Requirements.reserve(CINFO::Deps.size());
		for (auto it : CINFO::Deps)
			if(it != ComponentIDs::Invalid)
				ci.m_Requirements.push_back(static_cast<ComponentID>(it));
		ComponentInfo::RegisterComponentInfo(std::make_shared<ComponentInfo>(std::move(ci)));
	}
};

static const Register reg;

} //namespace TypeEditor
} //namespace MoonGlare

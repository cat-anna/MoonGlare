/*
  * Generated by cppsrc.sh
  * On 2016-09-12 17:58:33,31
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RectTransformComponent_H
#define RectTransformComponent_H

#include <Core/Component/AbstractComponent.h>

#include "../Margin.h"
#include "../Enums.h"
#include "../Rect.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

using namespace Core::Component;

union RectTransformComponentEntryFlagsMap {
	struct MapBits_t {
		bool m_Valid : 1;
		bool m_Dirty : 1;
	};
	MapBits_t m_Map;
	uint8_t m_UintValue;

	void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
	void ClearAll() { m_UintValue = 0; }

	static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

struct RectTransformComponentEntry {
	Handle m_SelfHandle;
	Entity m_OwnerEntity;
	RectTransformComponentEntryFlagsMap m_Flags;

	uint16_t m_Z;
	Point m_Position;				//not pod
	Point m_Size;					//not pod
	DEFINE_COMPONENT_PROPERTY(Z);
	DEFINE_COMPONENT_PROPERTY(Position);
	DEFINE_COMPONENT_PROPERTY(Size);
	//TODO: margin property
	//TODO: AlignMode property

	Margin m_Margin;				//not pod
	AlignMode m_AlignMode;

	math::mat4 m_GlobalMatrix;		//not pod
	math::mat4 m_LocalMatrix;		//not pod
	Rect m_ScreenRect;				//not pod

	Configuration::RuntimeRevision m_Revision;

	void Recalculate(RectTransformComponentEntry &Parent);

	void SetDirty() { m_Revision = 0; m_Flags.m_Map.m_Dirty = true; }

	void Reset() {
		m_Revision = 0;
	}
};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

struct RectTransformSettingsFlagsMap {
	struct MapBits_t {
		bool m_UniformMode : 1;
	};
	MapBits_t m_Map;
	uint8_t m_UintValue;

	void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
	void ClearAll() { m_UintValue = 0; }

	static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

class RectTransformComponent 
	: public TemplateStandardComponent<RectTransformComponentEntry, ComponentID::RectTransform>
	, public Core::iCustomDraw {
public:
	static constexpr char *Name = "RectTransform";
	static constexpr bool PublishID = true;

 	RectTransformComponent(ComponentManager *Owner);
 	virtual ~RectTransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool LoadComponentConfiguration(pugi::xml_node node) override;

	RectTransformComponentEntry &GetRootEntry() { return m_Array[0]; }

	bool IsUniformMode() const { return m_Flags.m_Map.m_UniformMode; }
	const Renderer::VirtualCamera& GetCamera() const { return m_Camera; }

	bool FindChildByPosition(Handle Parent, math::vec2 pos, Entity &eout);

	static void RegisterScriptApi(ApiInitializer &root);
	static void RegisterDebugScriptApi(ApiInitializer &root);

	static int EntryIndex(lua_State *lua);
	static int EntryNewIndex(lua_State *lua);
	static constexpr LuaMetamethods EntryMetamethods = { &EntryIndex , &EntryNewIndex, };

	static int FindChild(lua_State *lua);
protected:
	ScriptComponent *m_ScriptComponent;
	RectTransformSettingsFlagsMap m_Flags;
	Configuration::RuntimeRevision m_CurrentRevision;
	Point m_ScreenSize;
	Renderer::VirtualCamera m_Camera;

private:

//debug:
protected:
	// iCustomDraw
	virtual void D2Draw(Graphic::cRenderDevice& dev) override;
	Graphic::Shaders::Shader *m_Shader = nullptr;
};

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

#endif

/*
  * Generated by cppsrc.sh
  * On 2016-08-21 12:19:52,68
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef LightComponent_H
#define LightComponent_H

#include <Core/Component/AbstractComponent.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

using namespace Core::Component;

struct LightComponentEntry {
	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Active : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	FlagsMap m_Flags;
	Entity m_Owner;
	Handle m_SelfHandle;

	Light::LightBase m_Base;
	Light::LightType m_Type;

	Light::LightAttenuation m_Attenuation;
	float m_CutOff;

	void Reset() { }

	void SetActive(bool v) { m_Flags.m_Map.m_Active = v; }
	bool GetActive() const { return m_Flags.m_Map.m_Active; }
	void SetCastShadows(bool v) { m_Base.m_Flags.m_CastShadows = v; }
	bool GetCastShadows() const { return m_Base.m_Flags.m_CastShadows; }

};
//	static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
static_assert(std::is_pod<LightComponentEntry>::value, "LightComponentEntry must be pod!");

class LightComponent 
	: public TemplateStandardComponent<LightComponentEntry, ComponentID::Light> {
public:
	LightComponent(ComponentManager *Owner);
	virtual ~LightComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool Create(Entity Owner, Handle &hout) override;

	using LightEntry = ComponentEntry;

	static void RegisterScriptApi(ApiInitializer &root);
private:
	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;
};
} //namespace Component 
} //namespace Renderer 
} //namespace MoonGlare 

#endif

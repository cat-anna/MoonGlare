#pragma once

#include <Core/Component/TemplateStandardComponent.h>
#include <Renderer/Light.h>

namespace MoonGlare::Component {
using namespace Core::Component;

class TransformComponent;

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

	Renderer::LightBase m_Base;
	Renderer::LightType m_Type;
	Renderer::LightAttenuation m_Attenuation;

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
	: public TemplateStandardComponent<LightComponentEntry, SubSystemId::Light> {
public:
	LightComponent(SubsystemManager *Owner);
	virtual ~LightComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

	using LightEntry = ComponentEntry;

	static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
private:
	template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
	TransformComponent *m_TransformComponent;
};

}

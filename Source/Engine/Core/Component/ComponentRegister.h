/*
  * Generated by cppsrc.sh
  * On 2015-08-20 17:27:44,24
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ComponentRegister_H
#define ComponentRegister_H

namespace MoonGlare {
namespace Core {
namespace Component {

struct ComponentRegister {

	using ComponentCreateFunc = std::unique_ptr<AbstractComponent>(*)(ComponentManager*);
	struct ComponentInfo {
		ComponentID m_CID;
		ComponentCreateFunc m_CreateFunc;
		const char *m_Name;
		struct {
			bool m_RegisterID : 1;
		} m_Flags;
		void(*m_ApiRegFunc)(ApiInitializer &api); 
		int(*m_GetCID)();
	};

	using MapType = std::unordered_map < std::string, const ComponentInfo* >;
	static const ComponentInfo* GetComponentInfo(const char *Name) {
		auto it = s_ComponentMap->find(Name);
		if (it == s_ComponentMap->end())
			return nullptr;
		return it->second;
	}
	static const ComponentInfo* GetComponentInfo(ComponentID cid) {
		for (auto &it : *s_ComponentMap)
			if (it.second->m_CID == cid)
				return it.second;
		return nullptr;
	}
	static bool GetComponentID(const char *Name, ComponentID &cidout) {
		auto *ci = GetComponentInfo(Name);
		if (!ci)
			return false;
		cidout = ci->m_CID;
		return true;
	}
	static const MapType& GetComponentMap() { return *s_ComponentMap; }
	static void Dump(std::ostream &out);

	static bool ExtractCIDFromXML(pugi::xml_node node, ComponentID &out);

	static int RegisterComponentApi(ApiInitializer &api);
protected:
	static void SetComponent(const ComponentInfo *ci) {
		if (!s_ComponentMap)
			s_ComponentMap = new MapType();
		(*s_ComponentMap)[ci->m_Name] = ci;
	}
private:
	static MapType *s_ComponentMap;
};

template<class COMPONENT>
struct RegisterComponentID : public ComponentRegister {
	RegisterComponentID(const char *Name, bool PublishToLua = true, void(*ApiRegFunc)(ApiInitializer &api) = nullptr) {
		m_ComponentInfo.m_Name = Name;
		m_ComponentInfo.m_Flags.m_RegisterID = PublishToLua;
		m_ComponentInfo.m_ApiRegFunc = ApiRegFunc;
		m_ComponentInfo.m_CID = COMPONENT::GetComponentID();
		m_ComponentInfo.m_CreateFunc = &Construct;
		m_ComponentInfo.m_GetCID = &GetCID;
		SetComponent(&m_ComponentInfo);
	}
private:
	static std::unique_ptr<AbstractComponent> Construct(ComponentManager* cm) { return std::make_unique<COMPONENT>(cm); }
	static int GetCID() { 
		static_assert(sizeof(int) == sizeof(ComponentID), "Component id size does not match int size");
		return static_cast<int>(COMPONENT::GetComponentID());
	}
	static ComponentInfo m_ComponentInfo;
};

template<class COMPONENT>
ComponentRegister::ComponentInfo RegisterComponentID<COMPONENT>::m_ComponentInfo;

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

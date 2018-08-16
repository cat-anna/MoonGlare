#pragma once

#include <libSpace/src/Container/StaticVector.h>
#include "ComponentRegister.h"

#include <Foundation/Component/iSubsystem.h>

#include "nfComponent.h"
#include "SubsystemManager.h"

namespace MoonGlare::Core::Component {

using namespace MoonGlare::Component;

class AbstractSubsystem : public iSubsystem {
public:
    AbstractSubsystem(SubsystemManager *Owner) : m_Owner(Owner) { }

    void Step(const SubsystemUpdateData &conf) override {
        Step(reinterpret_cast<const MoveConfig&>(conf));
    };
    
    virtual void Step(const MoveConfig &conf) { }

    SubsystemManager* GetManager() { return m_Owner; }
private:
    SubsystemManager * m_Owner;
};

template<typename ELEMENT, ComponentId CID, size_t BUFFER = MoonGlare::Configuration::Storage::ComponentBuffer>
class TemplateStandardComponent 
	: public AbstractSubsystem
	, public ComponentIdWrap<CID> {
public:
	using ComponentEntry = ELEMENT;

	TemplateStandardComponent(SubsystemManager * Owner) :AbstractSubsystem(Owner) {
        m_EntityMapper.Fill(ComponentIndex::Invalid);
    }

	ComponentEntry* GetEntry(Entity e) { 
        auto index = m_EntityMapper.GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return nullptr;
        return &m_Array[index];
    }
    ComponentEntry* GetEntry(ComponentIndex index) {
        return &m_Array[index];
    }
    ComponentIndex GetComponentIndex(Entity e) const { return m_EntityMapper.GetIndex(e); }

	bool PushEntryToLua(Entity e, lua_State * lua, int & luarets) override {
		auto *entry = GetEntry(e);
		if (!entry) {
            luarets = 0;
            return true;
		}

		luarets = 1;
		luabridge::Stack<ComponentEntry*>::push(lua, entry);

		return true;
	}
protected:
	template<class T> using Array = Space::Container::StaticVector<ELEMENT, BUFFER>;
	Array<ComponentEntry> m_Array;
	EntityArrayMapper<> m_EntityMapper;

	void TrivialReleaseElement(size_t Index) {
		auto lastidx = m_Array.Allocated() - 1;

		if (lastidx == Index) {
			auto &last = m_Array[lastidx];
			last.Reset();
            m_EntityMapper.SetIndex(last.m_Owner, ComponentIndex::Invalid);
		} else {
			auto &last = m_Array[lastidx];
			auto &item = m_Array[Index];

            m_EntityMapper.SetIndex(item.m_Owner, ComponentIndex::Invalid);
            m_EntityMapper.SetIndex(last.m_Owner, Index);

			std::swap(last, item);
			last.Reset();
		}
		m_Array.DeallocateLast();
	}
};

} 

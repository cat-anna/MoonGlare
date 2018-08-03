#pragma once

#include <libSpace/src/Container/StaticVector.h>
#include "ComponentRegister.h"

#include <Foundation/Component/iSubsystem.h>

namespace MoonGlare::Core::Component {

using namespace MoonGlare::Component;

class AbstractSubsystem : public iSubsystem {
public:
    AbstractSubsystem(SubsystemManager *Owner) : m_Owner(Owner) {
        m_HandleTable = Owner->GetWorld()->GetHandleTable();
    }

    virtual ~AbstractSubsystem() { }

    void Update(const SubsystemUpdateData &conf) override { 
        Step(reinterpret_cast<const MoveConfig&>(conf));
    };
    
    virtual void Step(const MoveConfig &conf) { }

    SubsystemManager* GetManager() { return m_Owner; }
protected:
    HandleTable * GetHandleTable() { return m_HandleTable; }

    template<typename ARRAY, class THIS>
    typename ARRAY::value_type* TemplateGetEntry(THIS *This, ARRAY & arr, Handle h) {
        HandleIndex hi;
        if (!GetHandleTable()->GetHandleIndex(This, h, hi)) {
            return nullptr;
        }
        return &arr[hi];
    }
private:
    SubsystemManager * m_Owner;
    HandleTable *m_HandleTable;
};

template<typename ELEMENT, ComponentID CID, size_t BUFFER = MoonGlare::Configuration::Storage::ComponentBuffer>
class TemplateStandardComponent 
	: public AbstractSubsystem
	, public ComponentIDWrap<CID> {
public:
	using ComponentEntry = ELEMENT;

	TemplateStandardComponent(SubsystemManager * Owner) :AbstractSubsystem(Owner) {}

	ComponentEntry* GetEntry(Handle h) { return TemplateGetEntry(this, m_Array, h); }
	ComponentEntry* GetEntry(Entity e) { return GetEntry(m_EntityMapper.GetHandle(e)); }

	bool GetInstanceHandle(Entity Owner, Handle & hout) override {
		auto h = m_EntityMapper.GetHandle(Owner);
		if (!GetHandleTable()->IsValid(this, h)) {
			return false;
		}
		hout = h;
		return true;
	}

	bool PushEntryToLua(Handle h, lua_State * lua, int & luarets) override {
		auto entry = GetEntry(h);
		if (!entry) {
			return true;
		}

		luarets = 1;
		luabridge::Stack<ComponentEntry*>::push(lua, entry);

		return true;
	}
protected:
	template<class T> using Array = Space::Container::StaticVector<ELEMENT, BUFFER>;
	Array<ComponentEntry> m_Array;
	Core::EntityMapper m_EntityMapper;

	void TrivialReleaseElement(size_t Index) {
		auto lastidx = m_Array.Allocated() - 1;

		if (lastidx == Index) {
			auto &last = m_Array[lastidx];
			GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
			last.Reset();
		} else {
			auto &last = m_Array[lastidx];
			auto &item = m_Array[Index];

			std::swap(last, item);

			if (!GetHandleTable()->SetHandleIndex(this, item.m_SelfHandle, Index)) {
				AddLogf(Error, "Failed to move component handle index!");
			}
			GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
			last.Reset();
		}
		m_Array.DeallocateLast();
	}
};

} 

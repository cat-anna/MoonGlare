#include <pch.h>
#include <MoonGlare.h>

#include "MeshComponent.h"

#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/Mesh/MeshResource.h>
#include <Renderer/Renderer.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<MeshComponent, MeshComponentEntry> MeshComponentTypeInfo;
RegisterComponentID<MeshComponent> MeshComponentReg("Mesh");

MeshComponent::MeshComponent(SubsystemManager * Owner) 
    : TemplateStandardComponent(Owner)
    , m_TransformComponent(nullptr)
{
}

MeshComponent::~MeshComponent() {
}

//------------------------------------------------------------------------------------------

struct MeshComponent::LuaWrapper {
    MeshComponent *component;
    //ScriptComponent *scriptComponent;

    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        index = component->GetComponentIndex(owner);
        //}
        if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted RectTransform component!");
        }
    }

    bool IsVisible() const { Check(); return component->GetEntry(index)->m_Flags.m_Map.m_Visible; }
    void SetVisible(bool v) { Check(); component->GetEntry(index)->m_Flags.m_Map.m_Visible = v; }


    void SetMeshHandle(Renderer::MeshResourceHandle h) { Check(); component->GetEntry(index)->meshHandle = h; }
    Renderer::MeshResourceHandle GetMeshHandle() const { Check(); return component->GetEntry(index)->meshHandle; }
    void SetMaterialHandle(Renderer::MaterialResourceHandle h) { Check(); component->GetEntry(index)->materialHandle = h; }
    Renderer::MaterialResourceHandle GetMaterialHandle() const { Check(); return component->GetEntry(index)->materialHandle; }
};

MoonGlare::Scripts::ApiInitializer MeshComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
    .beginClass<LuaWrapper>("MeshComponent")
        .addProperty("Visible", &LuaWrapper::IsVisible, &LuaWrapper::SetVisible)
        .addProperty("MeshHandle", &LuaWrapper::GetMeshHandle, &LuaWrapper::SetMeshHandle)
        .addProperty("MaterialHandle", &LuaWrapper::GetMaterialHandle, &LuaWrapper::SetMaterialHandle)
    .endClass()
    ;
}

int MeshComponent::PushToLua(lua_State *lua, Entity owner) {
    auto index = m_EntityMapper.GetIndex(owner);
    if (index == Component::ComponentIndex::Invalid)
        return 0;
    LuaWrapper lw{ this, owner, index, };
    luabridge::push<LuaWrapper>(lua, lw);
    return 1;
}

//------------------------------------------------------------------------------------------

bool MeshComponent::Initialize() {
    auto &ed = GetManager()->GetEventDispatcher();
    ed.Register<MoonGlare::Component::EntityDestructedEvent>(this);

//	m_Array.MemZeroAndClear();
    m_Array.fill(MeshEntry());

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }
    
    return true;
}

//------------------------------------------------------------------------------------------

void MeshComponent::HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event) {
    auto index = m_EntityMapper.GetIndex(event.entity);
    if (index >= m_Array.Allocated())
        return;

    m_Array[index].m_Flags.m_Map.m_Valid = false;
    m_EntityMapper.SetIndex(event.entity, ComponentIndex::Invalid);
}

//------------------------------------------------------------------------------------------

void MeshComponent::Step(const Core::MoveConfig &conf) {
    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = m_Array[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and continue
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        //if (!item.m_Flags.m_Map.m_Visible) {
        //    continue;
        //}

        auto tindex = m_TransformComponent->GetComponentIndex(item.m_Owner);
        if (tindex == ComponentIndex::Invalid) {
            //item.m_Flags.m_Map.m_Valid = false;
            //LastInvalidEntry = i;
            //++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        auto &tr = m_TransformComponent->GetTransform(tindex);
        if (item.meshHandle.deviceHandle) {//dirty valid check
            conf.deffered->Mesh(
                tr.matrix(),
                tr.translation(),
                item.meshHandle);
            continue;
        }
    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "MeshComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        ReleaseElement(LastInvalidEntry);
    }
}

//------------------------------------------------------------------------------------------

bool MeshComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    auto node = reader.node;
    std::string meshUri = node.child("Mesh").text().as_string("");
    std::string materialUri = node.child("Material").text().as_string("");
    if (meshUri.empty()) {
        AddLogf(Error, "Attempt to load nameless Mesh!");
        return false;
    }
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    if (meshUri.find("file://") == 0) {
        auto &mm = GetManager()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetMeshManager();
        if (!mm.LoadMesh(meshUri, materialUri, entry.meshHandle)) {
            AddLogf(Error, "Mesh load failed!");
            return false;
        }
    }
    else {
        AddLog(Error, fmt::format("Invalid mesh uri: '{}'", meshUri));
        return false;
    }

    entry.m_Owner = owner;

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Visible = node.child("Visible").text().as_bool(true);

    //m_EntityMapper.SetHandle(entry.m_Owner, ch);
    m_EntityMapper.SetIndex(owner, index);
   
    return true;
}

void MeshComponent::ReleaseElement(size_t Index) {
    auto lastidx = m_Array.Allocated() - 1;

    if (lastidx == Index) {
        auto &last = m_Array[lastidx];
        last.Reset();
    } else {
        auto &last = m_Array[lastidx];
        auto &item = m_Array[Index];

        std::swap(last, item);

        last.Reset();
    }
    m_Array.DeallocateLast();
}

bool MeshComponent::Create(Entity Owner) {
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    entry.m_Owner = Owner;
                            
    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_Visible = true;

    return true;
}

} //namespace Component 
} //namespace Renderer
} //namespace MoonGlare 

#include "ComponentArray.h"
#include "ComponentInfo.h"
#include "EntityManager.h"

namespace MoonGlare::Component {

ComponentArray::ComponentArray(InterfaceMap& ifaceMap)
    : PerfProducer(ifaceMap) {
    ifaceMap.GetObject(entityManager);

    storageStatus.fill({});

	auto name = fmt::format(std::string("ComponentArray_{}"), (void*)this);
    auto cid = AddChart(name);

    BaseComponentInfo::ForEachComponent([this, cid](auto cindex, const BaseComponentInfo::ComponentClassInfo& info) {
        AddSeries((unsigned)cindex, info.componentName, Unit::Count, cid);

        arrayMappers[cindex] = std::make_unique<EntityArrayMapper<>>();
        arrayMappers[cindex]->Clear();

        size_t capacity = info.infoPtr->GetDefaultCapacity();
        storageStatus[cindex] = {
            0,
            capacity,
            info.byteSize,
            &info,
        };

        size_t byteSize = info.byteSize * capacity;
        componentMemory[cindex] = Memory::make_aligned_array<uint8_t>(byteSize, 16);
        memset(componentMemory[cindex].get(), 0, byteSize);

        componentOwner[cindex] = ComponentOwnerArray(new Entity[capacity]);
        memset(componentOwner[cindex].get(), 0, capacity * sizeof(Entity));

        componentFlag[cindex] = ComponentFlagArray(new ComponentFlagSet[capacity]);
        memset(componentFlag[cindex].get(), 0, capacity * sizeof(ComponentFlags));
    });

    DumpStatus("AfterConstruction");
}

ComponentArray::~ComponentArray() {
    DumpStatus("BeforeDestruction");
    ReleaseAllComponents();
}

void ComponentArray::Step() {
    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo& info) {
        AddData((unsigned)cindex, (float)storageStatus[cindex].allocated);
    });

    //GCStep();
}

void ComponentArray::GCStep() {
    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo& info) {
        GCStep(cindex);
    });
}

void ComponentArray::GCStep(ComponentClassId ccid) {
    auto &status = storageStatus[ccid];
    if (status.allocated == 0)
        return;

    bool done = false;
    do {
        ComponentIndex start = static_cast<ComponentIndex>(rand() % status.allocated);

        constexpr size_t stepSize = 16;

        for (ComponentIndex i = start; i < status.allocated && i - start < stepSize; i = (ComponentIndex)(i + 1)) {
            auto entity = componentOwner[ccid][i];
            if (entityManager->IsValid(entity)) {
                done = true;
                continue;
            }

            RemoveComponent(ccid, entity, i);
        }
    }
    while (!done);
}

//-------------------------------------

void ComponentArray::SwapComponents(ComponentClassId cci, ComponentIndex a, ComponentIndex b) {
    if (a == b)
        return;
    
    auto aE = componentOwner[cci][a];
    auto aMem = GetComponentMemory(a, cci);

    auto bE = componentOwner[cci][b];
    auto bMem = GetComponentMemory(b, cci);

    arrayMappers[cci]->SetIndex(aE, b);
    arrayMappers[cci]->SetIndex(bE, a);
    componentOwner[cci][a] = bE;
    componentOwner[cci][b] = aE;

    storageStatus[cci].info->swap(bMem, aMem);
}

void ComponentArray::RemoveComponent(ComponentClassId cci, Entity e, ComponentIndex index) {
    auto lastIndex = storageStatus[cci].Last();
    if (lastIndex != index)
        SwapComponents(cci, index, lastIndex);

    auto mem = GetComponentMemory(lastIndex, cci);
    storageStatus[cci].info->destructor(mem);

    arrayMappers[cci]->ClearIndex(e);

    storageStatus[cci].Deallocate();
}

//-------------------------------------

int ComponentArray::PushToScript(iSubsystemManager* manager, Entity e, ComponentClassId cci, lua_State* lua)
{
    auto index = arrayMappers[cci]->GetIndex(e);
    if (index == ComponentIndex::Invalid) {
        return 0;
    }
    auto sp = storageStatus[cci].info->scriptPush;
    if (!sp)
        return 0;
    sp(this, manager, e, lua);
}

//-------------------------------------

void ComponentArray::ReleaseComponents(ComponentClassId cci)
{
    if ((size_t)cci >= Configuration::MaxComponentTypes)
        return;
    auto& sst = storageStatus[cci];
    if (sst.capacity == 0)
        return;

    for (size_t index = 0; index < sst.allocated; ++index) {
        auto* memory = GetComponentMemory(static_cast<ComponentIndex>(index), cci);
        sst.info->destructor(memory);
    }
    sst.allocated = 0;
}

void ComponentArray::ReleaseAllComponents()
{
    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo& info) {
        ReleaseComponents(cindex);
    });
}

//-------------------------------------

void ComponentArray::DumpStatus(const char* id) const
{
    for (size_t index = 0; index < storageStatus.size(); ++index) {
        auto& sst = storageStatus[(ComponentClassId)index];
        if (!sst.info)
            continue;
        float mem = sst.capacity * (sst.elementByteSize + sizeof(Entity) + sizeof(ComponentFlags)) + sizeof(EntityArrayMapper<>);
        mem /= 1024.0f;
        AddLogf(Debug, "ComponentArray [%p:%s] %02u/%02u %20s : size: %3u bytes; used:%6u cap:%6u  mem:%5.1fkib",
            this, id ? id : "?", index, storageStatus.size() - 1, sst.info->componentName, sst.elementByteSize, sst.allocated, sst.capacity,
            mem);
    }
}

}

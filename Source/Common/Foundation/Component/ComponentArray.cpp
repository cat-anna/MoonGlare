#include "ComponentArray.h"  
#include "ComponentInfo.h"

namespace MoonGlare::Component {

ComponentArray::ComponentArray() {
    storageStatus.fill({});

    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo &info) {
        arrayMappers[cindex] = std::make_unique<EntityArrayMapper<>>();
        arrayMappers[cindex]->Clear();
        
        size_t capacity = info.infoPtr->GetDefaultCapacity();
        storageStatus[cindex] = {
            0, 
            capacity,
            info.byteSize, 
            &info,
        };

        size_t byteSize = info.byteSize * storageStatus[cindex].capacity;
        componentMemory[cindex] = ComponentMemory(new char[byteSize]);
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

//-------------------------------------

int ComponentArray::PushToScript(iSubsystemManager *manager, Entity e, ComponentClassId cci, lua_State *lua) {
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

void ComponentArray::ReleaseComponents(ComponentClassId cci) {
    if ((size_t)cci >= Configuration::MaxComponentTypes)
        return;
    auto &sst = storageStatus[cci];
    if (sst.capacity == 0)
        return;

    for (size_t index = 0; index < sst.allocated; ++index) {
        auto *memory = GetComponentMemory(static_cast<ComponentIndex>(index), cci);
        sst.info->destructor(memory);
    }
    sst.allocated = 0;
}

void ComponentArray::ReleaseAllComponents() {
    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo &info) {
        ReleaseComponents(cindex);
    });
}

//-------------------------------------

void ComponentArray::DumpStatus(const char *id) const {
    for (size_t index = 0; index < storageStatus.size(); ++index) {
        auto &sst = storageStatus[(ComponentClassId)index];
        if (!sst.info)
            continue;
        float mem = sst.capacity * (sst.elementByteSize + sizeof(Entity) + sizeof(ComponentFlags)) + sizeof(EntityArrayMapper<>);
        mem /= 1024.0f;
        AddLogf(Debug, "ComponentArray [%p:%s] %02u/%02u %20s : size: %3u bytes; used:%6u cap:%6u  mem:%5.1fkib", 
            this, id ? id : "?", index, storageStatus.size()-1, sst.info->componentName, sst.elementByteSize, sst.allocated, sst.capacity,
            mem);
    }
}

}

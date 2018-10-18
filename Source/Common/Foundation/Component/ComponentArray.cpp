#include "ComponentArray.h"  
#include "ComponentInfo.h"

namespace MoonGlare::Component {

ComponentArray::ComponentArray() {
    storageStatus.fill({});

    BaseComponentInfo::ForEachComponent([this](auto cindex, const BaseComponentInfo::ComponentClassInfo &info) {
        auto index = static_cast<size_t>(cindex);
        arrayMappers[index] = std::make_unique<EntityArrayMapper<>>();      
        arrayMappers[index]->Clear();
        
        size_t capacity = info.infoPtr->GetDefaultCapacity();
        storageStatus[index] = {
            0, 
            capacity,
            info.byteSize, 
            &info,
        };

        size_t byteSize = info.byteSize * storageStatus[index].capacity;
        componentMemory[index] = ComponentMemory(new char[byteSize]);
        memset(componentMemory[index].get(), 0, byteSize);

        componentOwner[index] = ComponentOwner(new Entity[capacity]);
        memset(componentOwner[index].get(), 0, capacity * sizeof(Entity));
    });

    DumpStatus("AfterConstruction");
}                           

ComponentArray::~ComponentArray() {
    DumpStatus("BeforeDestruction");
    ReleaseAllComponents();
}

//-------------------------------------

#if 0
bool ComponentArray::AllocatePage(unsigned pageIndex) {
    if (pageMemory[pageIndex])
        return true;

    PerComponentArray<int> offsets;
    offsets.fill(-1);

    int offset = 0;
    for (ComponentClassId cindex = 0; cindex < offsets.size(); ++cindex) {
        auto &info = BaseComponentInfo::GetComponentTypeInfo(cindex);

        if (!info.infoPtr)
            continue;

        if (offset & 0xF) {
            offset &= ~0xF;
            offset += 0x10;
        }

        offsets[cindex] = offset;
        offset += Configuration::ComponentsPerPage * info.byteSize;
    }

    pageMemory[pageIndex].reset(new char[offset]);
    char *mem = pageMemory[pageIndex].get();

    auto &page = componentPageArray[pageIndex];
    for (size_t cindex = 0; cindex < offsets.size(); ++cindex) {
        if(offsets[cindex] >= 0)
            page[cindex] = mem + offsets[cindex];
    }

    return true;
}
#endif

//-------------------------------------

void ComponentArray::ReleaseComponents(ComponentClassId cci) {
    if ((size_t)cci >= Configuration::MaxComponentTypes)
        return;
    size_t index = static_cast<size_t>(cci);
    auto &sst = storageStatus[index];
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
        auto &sst = storageStatus[index];
        if (!sst.info)
            continue;
        AddLogf(Debug, "ComponentArray [%p:%s] %02u/%02u %20s : size: %3u bytes; used:%3u cap:%3u  mem:%u", 
            this, id ? id : "?", index, storageStatus.size()-1, sst.info->componentName, sst.elementByteSize, sst.allocated, sst.capacity,
            sst.capacity * (sst.elementByteSize + sizeof(Entity)) + sizeof(EntityArrayMapper<>));
    }
}

}

#include "ComponentArray.h"  

namespace MoonGlare::Component {

ComponentArray::ComponentArray() {
    componentPageArray.fill({});
    componentValidArray.fill(0);

    //if (!AllocatePage(0))
        //throw std::runtime_error("Cannot allocate component page 0!");
}

ComponentArray::~ComponentArray() {
    ReleaseAllComponents();
}

//-------------------------------------

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

//-------------------------------------

void ComponentArray::ReleaseComponents(Index index) {
    if (index >= Configuration::MaxComponent)
        return;
    if (componentValidArray[index] == 0)
        return;

    for (ComponentClassId cindex = 0; cindex < Configuration::MaxComponentTypes; ++cindex) {
        auto &info = BaseComponentInfo::GetComponentTypeInfo(cindex);
        if (!info.infoPtr)
            continue;

        auto[page, offset] = IndexToPage(index);
        void *componentPageMemory = componentPageArray[page][cindex];
        assert(componentPageMemory);
        char *ptr = reinterpret_cast<char*>(componentPageMemory) + offset * info.byteSize;
        info.destructor(ptr);
    }
    componentValidArray[index] = 0;
}

void ComponentArray::ReleaseAllComponents() {
    for (ComponentClassId index = 0; index < Configuration::MaxComponent; ++index) {
        ReleaseComponents(index);
    }
}

}

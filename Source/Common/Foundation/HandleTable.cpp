#include "HandleTable.h"

namespace MoonGlare {

HandleTable::HandleTable() {
    handlePrivateData.fill(0);
    allocator.Clear();
#ifdef DEBUG
    generationbuffer.Fill(1);
#else
    generationbuffer.FillRandom();
#endif // DEBUG

}

HandleTable::~HandleTable() { }

Handle HandleTable::Allocate(HandlePrivateData value) {
    uint32_t index;
    if (!allocator.Allocate(index)) {
        AddLog(Error, "Allocation failed!");
        __debugbreak();
        return {};
    }

    Handle hout;
    hout.SetIndex(index);
    hout.SetGeneration(generationbuffer.Generation(index));
    handlePrivateData[index] = value;

    return hout;
}
           
void HandleTable::Release(Handle h) {
    if (!IsValid(h)) 
        return;
    auto index = h.GetIndex();

    generationbuffer.NewGeneration(index);
    allocator.Release(index);
}

bool HandleTable::GetHandleData(Handle h, HandlePrivateData &value) {
    if (!IsValid(h))
        return false;
    value = handlePrivateData[h.GetIndex()];
    return true;
}

bool HandleTable::SetHandleData(Handle h, HandlePrivateData value) {
    if (!IsValid(h))
        return false;
    handlePrivateData[h.GetIndex()] = value;
    return true;
}

} //namespace MoonGlare 

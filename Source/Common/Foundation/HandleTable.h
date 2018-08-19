#pragma once

#include <Foundation/Memory/StaticGenerationBuffer.h>
#include <Foundation/Memory/StaticIndexQueue.h>

#include "Handle.h"

namespace MoonGlare {

class HandleTable final {
public:
 	HandleTable();
 	~HandleTable();

	using HandlePrivateData = uint64_t;

    bool IsValid(Handle h) const {
        auto index = h.GetIndex();
        auto generation = h.GetGeneration();
        return generation == generationbuffer.Generation(index);
    }

	void Release(Handle h);
    Handle Allocate(HandlePrivateData value = 0);

	bool GetHandleData(Handle h, HandlePrivateData &value);
	bool SetHandleData(Handle h, HandlePrivateData value);
protected:
	template<class T> using Array = std::array<T, Configuration::HandleIndexLimit>;

    Array<HandlePrivateData> handlePrivateData;
    Memory::GenerationBuffer<Handle::Generation_t, Configuration::HandleIndexLimit> generationbuffer;
    Memory::StaticIndexQueue<Handle::Index_t, Configuration::HandleIndexLimit> allocator;
};

} //namespace MoonGlare 

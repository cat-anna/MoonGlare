#pragma once

#include <array>
#include "Entity.h"

namespace MoonGlare::Component
{

template<typename IndexType, size_t SizeLimit>
struct EntityArrayMapper {
    static_assert(std::is_integral_v<IndexType> || std::is_enum_v<IndexType>);

    using Array = std::array<IndexType, SizeLimit>;

    void Fill(IndexType t) { mapper.fill(t); }
    IndexType GetIndex(Entity e) const { return mapper[e.GetIndex()]; }
    void SetIndex(Entity e, IndexType h) { mapper[e.GetIndex()] = h; }
    void SetIndex(Entity e, size_t h) { mapper[e.GetIndex()] = (IndexType)h; }
    void Swap(Entity a, Entity b) {
        auto v = mapper[a.GetIndex()];
        mapper[a.GetIndex()] = mapper[b.GetIndex()];
        mapper[b.GetIndex()] = v;
    }
protected:
    Array mapper;
};

static_assert(std::is_pod_v<EntityArrayMapper<uint16_t, 1024>>);

}

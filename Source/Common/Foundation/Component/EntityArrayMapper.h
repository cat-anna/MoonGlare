#pragma once

#include <array>
#include "Configuration.h"
#include "Entity.h"

namespace MoonGlare::Component {

template<
    size_t SizeLimit = Configuration::EntityLimit, 
    typename IndexType = ComponentIndex, 
    IndexType InvalidValue = ComponentIndex::Invalid>
struct EntityArrayMapper {
    static_assert(std::is_integral_v<IndexType> || std::is_enum_v<IndexType>);

    using Array = std::array<IndexType, SizeLimit>;
    static constexpr IndexType InvalidIndex = InvalidValue;


    void Clear() { Fill(InvalidIndex); }
    void Fill(IndexType t) { mapper.fill(t); }
    IndexType GetIndex(Entity e) const { return mapper[e.GetIndex()]; }
    void ClearIndex(Entity e) { mapper[e.GetIndex()] = InvalidIndex; }
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

static_assert(std::is_pod_v<EntityArrayMapper<1024>>);

}

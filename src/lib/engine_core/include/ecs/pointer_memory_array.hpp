#pragma once

#include <cstdint>
#include <memory>

namespace MoonGlare::ECS {

template <typename Element_t>
struct PointerMemory {
public:
    using Element = Element_t;

    PointerMemory(){};
    PointerMemory(size_t element_count)
        : memory(std::make_unique<Element_t[]>(element_count)), element_count(element_count){};

    PointerMemory(PointerMemory &&) = default;
    ~PointerMemory() = default;
    PointerMemory &operator=(PointerMemory &&) = default;

    void *ptr() const { return memory.get(); }
    Element_t *get() const { return memory.get(); }
    size_t size() const { return element_count; }

    Element_t *reset(size_t element_count) {
        memory = std::make_unique<Element_t[]>(element_count);
        this->element_count = element_count;
        return get();
    }

private:
    std::unique_ptr<Element_t[]> memory;
    size_t element_count;
};

using RawMemory = PointerMemory<uint8_t>;

} // namespace MoonGlare::ECS

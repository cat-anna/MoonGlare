#pragma once

namespace MoonGlare::Component::Configuration {

static constexpr uint32_t MaxComponentTypes = 16;
using ComponentTypeBitSet = uint16_t;
static_assert(MaxComponentTypes <= sizeof(ComponentTypeBitSet) * 8);

static constexpr uint32_t ComponentsPerPage = 256;
static constexpr uint32_t MaxComponentPages = 8;

static constexpr uint32_t MaxComponent = MaxComponentPages * ComponentsPerPage;

//------

static constexpr uint32_t MaxEventTypes = 16;

//------

static constexpr uint32_t EventDispatcherQueueSize = 1 * 1024; //bytes

}

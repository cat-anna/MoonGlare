#pragma once

#include <nlohmann/json.hpp>
#include <xxhash.h>

namespace MoonGlare {

using FileResourceId = XXH64_hash_t;
static constexpr FileResourceId kInvalidResourceId = 0;

using ResourceHandle = uint64_t;
constexpr ResourceHandle kInvalidResourceHandle = 0;

enum class ResourceType {
    kUnknown,
    kTexture,
    kShader,
};

struct BasicRuntimeResourceHandle {
    FileResourceId resource_id; //serialized to/from json
    ResourceHandle loaded_handle;
};

template <ResourceType ResourceType_t>
struct RuntimeResourceHandle : public BasicRuntimeResourceHandle {
    static constexpr ResourceType kType = ResourceType_t;
};

using TextureRuntimeResourceHandle = RuntimeResourceHandle<ResourceType::kTexture>;
using ShaderRuntimeResourceHandle = RuntimeResourceHandle<ResourceType::kShader>;

class iRuntimeResourceLoader {
public:
    virtual ~iRuntimeResourceLoader() = default;

    void LoadResource(TextureRuntimeResourceHandle &res) {
        res.loaded_handle = LoadTextureResource(res.resource_id);
    }
    void LoadResource(ShaderRuntimeResourceHandle &res) {
        res.loaded_handle = LoadShaderResource(res.resource_id);
    }

    virtual ResourceHandle LoadTextureResource(FileResourceId file_id) = 0;
    virtual ResourceHandle LoadShaderResource(FileResourceId file_id) = 0;
};

template <ResourceType ResourceType_t>
inline void to_json(nlohmann::json &j, const RuntimeResourceHandle<ResourceType_t> &p) {
    j = p.resource_id;
}

template <ResourceType ResourceType_t>
inline void from_json(const nlohmann::json &j, RuntimeResourceHandle<ResourceType_t> &p) {
    p = {j.get<FileResourceId>(), kInvalidResourceHandle};
}

} // namespace MoonGlare

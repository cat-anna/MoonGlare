#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <xxhash.h>

namespace MoonGlare::StarVfs {

using FilePathHash = XXH64_hash_t;
using FileContentHash = XXH64_hash_t;
using ContainerFileId = XXH64_hash_t;

struct Hasher {
    static constexpr unsigned long long kSeed = 0;

    Hasher() { XXH64_reset(state.get(), kSeed); }

    void update(const void *data, size_t size) {
        if (size > 0) {
            XXH64_update(state.get(), data, size);
        }
    }
    XXH64_hash_t get() { return XXH64_digest(state.get()); }
    Hasher clone() const {
        Hasher r;
        XXH64_copyState(r.state.get(), state.get());
        return r;
    }

    static XXH64_hash_t Hash(const void *data, size_t size) { return XXH64(data, size, kSeed); }
    static XXH64_hash_t Hash(const std::string &str) { return Hash(str.c_str(), str.size()); }
    static XXH64_hash_t Hash(const std::string_view &str) { return Hash(str.data(), str.size()); }

private:
    struct FreeInstance {
        void operator()(XXH64_state_t *h) const { XXH64_freeState(h); }
    };
    using ptr_t = std::unique_ptr<XXH64_state_t, FreeInstance>;
    ptr_t state{XXH64_createState()};
};

} // namespace MoonGlare::StarVfs

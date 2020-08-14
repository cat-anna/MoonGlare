#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <xxhash.h>

namespace MoonGlare::StarVfs {

using FilePathHash = XXH64_hash_t;
using FileResourceId = XXH64_hash_t;
using ContainerFileId = XXH64_hash_t;

struct Hasher {
    using SeedType = unsigned long long;
    static constexpr SeedType kDefaultSeed = 0;

    Hasher(SeedType seed = kDefaultSeed) { XXH64_reset(state.get(), seed); }

    void update(const void *data, size_t size) {
        if (size > 0) {
            XXH64_update(state.get(), data, size);
        }
    }

    void update(const std::string &str) { update(str.c_str(), str.size()); }
    void update(const std::string_view &str) { update(str.data(), str.size()); }
    void update(const char *str) { update(str, strlen(str)); }

    XXH64_hash_t get() { return XXH64_digest(state.get()); }
    Hasher clone() const {
        Hasher r;
        XXH64_copyState(r.state.get(), state.get());
        return r;
    }

    static XXH64_hash_t Hash(const void *data, size_t size, SeedType seed = kDefaultSeed) {
        return XXH64(data, size, seed);
    }
    static XXH64_hash_t Hash(const std::string &str, SeedType seed = kDefaultSeed) {
        return Hash(str.c_str(), str.size(), seed);
    }
    static XXH64_hash_t Hash(const std::string_view &str, SeedType seed = kDefaultSeed) {
        return Hash(str.data(), str.size(), seed);
    }
    template <typename... ARGS> static XXH64_hash_t HashTogether(SeedType seed, const ARGS &... args) {
        Hasher h{seed};
        (h.update(args), ...);
        return h.get();
    }
    template <typename... ARGS> static XXH64_hash_t HashTogether(const ARGS &... args) {
        Hasher h;
        (h.update(args), ...);
        return h.get();
    }

private:
    struct FreeInstance {
        void operator()(XXH64_state_t *h) const { XXH64_freeState(h); }
    };
    using ptr_t = std::unique_ptr<XXH64_state_t, FreeInstance>;
    ptr_t state{XXH64_createState()};
};

} // namespace MoonGlare::StarVfs

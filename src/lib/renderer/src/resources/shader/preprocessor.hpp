
#pragma once

#include "readonly_file_system.h"
#include "shader_loader_interface.hpp"
#include <array>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace MoonGlare::Renderer::Resources::Shader {

class ShaderFileCache : public iSharderPreprocesorCache {
public:
    ShaderFileCache(iReadOnlyFileSystem *fs) : file_system(fs){};
    ~ShaderFileCache() override = default;

    void ReadFile(const std::string &file_name, const ReadBuffer *&out) override;
    void Clear() override;

private:
    std::unordered_map<std::string, std::unique_ptr<ReadBuffer>> loaded_files;
    iReadOnlyFileSystem *const file_system;
};

struct OutputBuffer : public std::list<std::string> {
    template <typename... ARGS>
    void pushf(const char *fmt, ARGS &&...args) {
        push_back(fmt::sprintf(fmt, std::forward<ARGS>(args)...));
    }
    void pushs(const char *fmt, const std::string &str) { pushf(fmt, str.c_str()); }
    template <typename... ARGS>
    void push(const char *fmt, ARGS... args) {
        push_back(fmt::format(fmt, std::forward<ARGS>(args)...));
    }
    void push(std::string l) { push_back(std::move(l)); }
};

class Preprocessor final : public iSharderPreprocesor {
public:
    Preprocessor(iSharderPreprocesorCache *cache) : file_cache(cache){};
    ~Preprocessor() override = default;

    void Clear();

    void Version(const std::string &value) override { version = value; }
    void Define(const std::string &id, const std::string &value) override { defines[id] = value; }
    void Define(std::string id, const char *value) { defines[id] = value; }
    template <typename T>
    void Define(std::string id, T t) {
        Define(id, std::to_string(t));
    }

    void DefineString(std::string id, std::string value) { Define(id, '"' + value + '"'); }
    void DefineString(std::string id, const char *value) { DefineString(id, value); }
    template <typename T>
    void DefineString(std::string id, T t) {
        DefineString(id, std::to_string(t));
    }

    //iSharderPreprocesor
    std::string GetOutput() override;
    void ClearOutput() override;
    void PushFile(const std::string &name) override;

private:
    using ReadBuffer = ShaderFileCache::ReadBuffer;

    iSharderPreprocesorCache *const file_cache;
    std::unordered_map<std::string, std::string> defines;
    std::unordered_set<std::string> included_files;
    std::string version;

    OutputBuffer output_buffer;

    void Process(const std::string &file_name, int level);
    void GenerateDefines();

    using PreprocessorTokenHadler = void (Preprocessor::*)(const std::string &file_name,
                                                           const std::string &line, int level,
                                                           std::smatch match);

    void HandleIncludeToken(const std::string &file_name, const std::string &line, int level,
                            std::smatch match);

    struct PreprocessorToken {
        std::regex regExp;
        PreprocessorTokenHadler handler;
    };
    static const std::array<PreprocessorToken, 1> kKnownTokens;
};

} // namespace MoonGlare::Renderer::Resources::Shader

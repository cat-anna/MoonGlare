#pragma once

#include <array>
#include <fmt/format.h>
#include <functional>
#include <stdexcept>
#include <string>

namespace MoonGlare::Renderer::Resources::Shader {

class iSharderPreprocesorCache {
public:
    virtual ~iSharderPreprocesorCache() = default;

    using ReadBuffer = std::vector<std::string>;

    virtual void ReadFile(const std::string &file_name, const ReadBuffer *&out) = 0;
    virtual void Clear() = 0;
};

class iSharderPreprocesor {
public:
    virtual ~iSharderPreprocesor() = default;

    struct ParseException : public std::runtime_error {
        ParseException(const std::string &file_name, int level, const std::string &message)
            : runtime_error(message), file_name(file_name), include_level(level) {}
        const std::string file_name;
        const int include_level;
    };
    struct MissingFileException : public ParseException {
        MissingFileException(const std::string &file_name, int level)
            : ParseException(file_name, level, fmt::format("File {} is missing", file_name)) {}
    };

    virtual void PushFile(const std::string &name) = 0;
    virtual void ClearOutput() = 0;
    virtual std::string GetOutput() = 0;

    virtual void Version(const std::string &value) = 0;
    virtual void Define(const std::string &id, const std::string &value) = 0;
};

class iShaderCodeLoader {
public:
    virtual ~iShaderCodeLoader() = default;

    enum class ShaderType : uint8_t {
        Geometry,
        Vertex,
        Fragment,

        MaxValue,
    };

    struct ShaderFileInfo {
        ShaderType shader_type;
        std::string shader_ext;
        std::string name;
    };

    static const std::array<ShaderFileInfo, static_cast<size_t>(ShaderType::MaxValue)> kShaderFiles;

    struct ShaderCode {
        std::array<std::string, static_cast<size_t>(ShaderType::MaxValue)> code;

        operator bool() const {
            for (auto &item : code) {
                if (!item.empty()) {
                    return true;
                }
            }
            return false;
        }
    };

    virtual ShaderCode LoadCode(const std::string &Name) = 0;
    virtual void ScheduleLoadCode(const std::string &Name,
                                  std::function<void(ShaderCode)> result_cb) = 0;
};

} // namespace MoonGlare::Renderer::Resources::Shader

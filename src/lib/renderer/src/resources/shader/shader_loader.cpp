
#include "shader_loader.hpp"
#include "debug_dump.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Resources::Shader {

using ShaderType = ShaderCodeLoader::ShaderType;
using ShaderFileInfo = ShaderCodeLoader::ShaderFileInfo;

const std::array<ShaderFileInfo, static_cast<size_t>(ShaderType::MaxValue)>
    iShaderCodeLoader::kShaderFiles = {
        ShaderFileInfo{ShaderType::Vertex, "vs", "vertex"},
        ShaderFileInfo{ShaderType::Fragment, "fs", "fragment"},
        ShaderFileInfo{ShaderType::Geometry, "gs", "geometry"},
};

void ShaderCodeLoader::ScheduleLoadCode(const std::string &Name,
                                        std::function<void(ShaderCode)> result_cb) {
    async_loader->QueueTask(
        [this, cb = std::move(result_cb), name = Name]() { cb(LoadCode(name)); });
}

ShaderCodeLoader::ShaderCode ShaderCodeLoader::LoadCode(const std::string &name) {

    ShaderCode output;
    output.code.fill(std::string());

    auto TryLoad = [&](std::string fn, const ShaderFileInfo &shaderfile) -> bool {
        try {
            auto processor = preprocessor_factory();

            processor->Version("420");

            processor->Define(shaderfile.name + "_main", "main");
            processor->Define("shader_type", shaderfile.name);

            // buffer += "#define SHINESS_SCALER 128.0f\n";
            // buffer += "\n";
            // buffer += GenerateGaussianDisc(shaderConfiguration->gaussianDiscLength, shaderConfiguration->gaussianDiscRadius);
            // buffer += "\n";
            // buffer += "//preprocessed code begin\n";
            // buffer += code.m_Code[index];
            // buffer += "//preprocessed code end\n";
            // buffer += "\n";

            processor->PushFile(fn);

            auto index = static_cast<unsigned>(shaderfile.shader_type);

            auto code = processor->GetOutput();

            DebugDump(fmt::format("{}_{}.{}",
                                  std::filesystem::path(name).filename().generic_string(),
                                  shaderfile.name, shaderfile.shader_ext),
                      code);

            output.code[index].swap(code);
        } catch (const iSharderPreprocesor::ParseException &e) {
            if (e.include_level > 0) {
                AddLog(Error, fmt::format("Failure during preprocessing file {} for shader {} : {}",
                                          fn, name, e.what()));
                throw e;
            } else {
                // no code for sub-shader
                return false;
            }
        } catch (const std::exception &e) {
            AddLog(Error,
                   fmt::format("Unknown failure during preprocessing file {} for shader {} : {}",
                               fn, name, e.what()));
            throw e;
        }
        return true;
    };

    for (auto &shaderfile : kShaderFiles) {
        try {
            if (!TryLoad(name + "." + shaderfile.shader_ext, shaderfile)) {
                AddLog(Warning,
                       fmt::format("No dedicated file for {} shader {}", shaderfile.name, name));
            }
        } catch (const std::exception &) {
            AddLogf(Error, "Failed to load shader %s", name.c_str());
            return {};
        }
    }

    return output;
}

} // namespace MoonGlare::Renderer::Resources::Shader

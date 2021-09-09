
#pragma once

#include "async_loader.hpp"
#include "readonly_file_system.h"
#include "shader_loader_interface.hpp"

namespace MoonGlare::Renderer::Resources::Shader {

class ShaderCodeLoader final : public iShaderCodeLoader {
public:
    using PreprocessorFactory = std::function<std::unique_ptr<iSharderPreprocesor>()>;

    ShaderCodeLoader(iAsyncLoader *_async_loader, iReadOnlyFileSystem *fs,
                     PreprocessorFactory _preprocessor_factory)
        : async_loader(_async_loader), file_system(fs),
          preprocessor_factory(_preprocessor_factory) {}

    ~ShaderCodeLoader() override = default;

    ShaderCode LoadCode(const std::string &name) override;
    void ScheduleLoadCode(const std::string &Name,
                          std::function<void(ShaderCode)> result_cb) override;

private:
    iAsyncLoader *const async_loader;
    iReadOnlyFileSystem *const file_system;
    PreprocessorFactory preprocessor_factory;
};

} // namespace MoonGlare::Renderer::Resources::Shader

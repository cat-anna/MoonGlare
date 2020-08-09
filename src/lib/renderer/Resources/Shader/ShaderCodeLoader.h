
#pragma once
namespace MoonGlare::Renderer::Resources::Shader {

class ShaderCodeLoader {
public:
    virtual ~ShaderCodeLoader() {}

    enum class ShaderType : uint8_t {
        Geometry,
        Vertex,
        Fragment,

        MaxValue,
    };

    struct ShaderCode {
        std::array<std::string, static_cast<size_t>(ShaderType::MaxValue)> m_Code;
    };

    virtual bool LoadCode(const std::string &Name, ShaderCode &Output) {
        return false;
    };
};


}


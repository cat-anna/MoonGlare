
#include <Foundation/Math/Constants.h>

#include "../ResourceManager.h"
#include "../../Renderer.h"
#include "../../RenderDevice.h"

#include "../../Frame.h"
#include "../../Commands/CommandQueue.h"
#include "../../Commands/OpenGL/ArrayCommands.h"
#include "../../Commands/OpenGL/ShaderInitCommands.h"

#include "../AsyncLoader.h"

#include "ShaderResource.h"
#include "ShaderCodeLoader.h"
#include "Loader.h"

namespace MoonGlare::Renderer::Resources::Shader {

uint32_t ShaderHandlerInterface::s_InterfaceIndexAlloc = 0;

ShaderResource::ShaderResource() { }

ShaderResource::~ShaderResource() { }

void ShaderResource::Initialize(ResourceManager *Owner, iFileSystem *fileSystem) {
    assert(Owner);
    assert(fileSystem);

    m_ResourceManager = Owner;
    shaderConfiguration = &Owner->GetConfiguration()->shader;
    shaderCodeLoader = std::make_unique<Loader>(fileSystem);

    m_ShaderHandle.fill(Device::InvalidShaderHandle);
    m_ShaderInterface.fill(nullptr);
    Conf::UniformLocations locs;
    locs.fill(Device::InvalidShaderUniformHandle);
    m_ShaderUniform.fill(locs);
    m_ShaderName.fill(std::string());
    for(auto &item: m_ShaderLoaded)
        item = false;

}

void ShaderResource::Finalize() {
}

//---------------------------------------------------------------------------------------

void ShaderResource::ReloadAll() {
    assert(this);

    auto loader = dynamic_cast<AsyncLoader*>(m_ResourceManager->GetLoader()); //TODO: this is ugly
    for (auto i = 0u; i < m_ShaderLoaded.size(); ++i) {
        if (m_ShaderLoaded[i]) {
            DebugLogf(Warning, "Reloading shader %s", m_ShaderName[i].c_str());
            ShaderResourceHandleBase h;
            h.m_TmpGuard = h.GuardValue;
            h.m_Index = i;
            loader->SubmitShaderLoad(h);
        }
    }
}

bool ShaderResource::Reload(const std::string &Name) {
    for (auto index = 0u; index < m_ShaderName.size(); ++index) {
        if (m_ShaderName[index] == Name) {
            ShaderResourceHandleBase h;
            h.m_TmpGuard = h.GuardValue;
            h.m_Index = index;
            auto loader = dynamic_cast<AsyncLoader*>(m_ResourceManager->GetLoader()); //TODO: this is ugly
            loader->SubmitShaderLoad(h);
            return true;
        }
    }
    return false;
}

bool ShaderResource::GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, ShaderResourceHandleBase handle) {
    assert(this);
    if (handle.m_TmpGuard != handle.GuardValue) {
        AddLogf(Error, "Invalid handle!");
        return false; 
    }
    return GenerateReload(queue, Memory, handle.m_Index);
}

bool ShaderResource::GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);
    if (!m_ShaderLoaded[ifindex])
        return false;
    return 
        ReleaseShader(queue, Memory, ifindex) &&
        GenerateLoadCommand(queue, Memory, ifindex) &&
        InitializeUniforms(queue, Memory, ifindex) &&
        InitializeSamplers(queue, Memory, ifindex) ;
}

//---------------------------------------------------------------------------------------

ShaderResourceHandleBase ShaderResource::LoadShader(const std::string & ShaderName, ShaderHandlerInterface * ShaderIface) {
    assert(this);
    assert(ShaderIface);

    ShaderResourceHandleBase out;

    auto ifindex = ShaderIface->InterfaceID();

    bool isalloc = false;
    if (!m_ShaderLoaded[ifindex].compare_exchange_strong(isalloc, true)) {
        out.m_TmpGuard = out.GuardValue;
        out.m_Index = static_cast<uint16_t>(ifindex);
        return out;
    }

    m_ShaderName[ifindex] = ShaderName;
    m_ShaderHandle[ifindex] = Device::InvalidShaderHandle;
    m_ShaderUniform[ifindex].fill(Device::InvalidShaderUniformHandle);
    m_ShaderInterface[ifindex] = ShaderIface;

    out.m_TmpGuard = out.GuardValue;
    out.m_Index = static_cast<uint16_t>(ifindex);

    auto loader = dynamic_cast<AsyncLoader*>(m_ResourceManager->GetLoader()); //TODO: this is ugly
    loader->SubmitShaderLoad(out);

    return out;
}

//---------------------------------------------------------------------------------------

bool ShaderResource::ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);
    auto *arg = q.PushCommand<Commands::ReleaseShaderResource>();
    arg->m_ShaderHandle = &m_ShaderHandle[ifindex];
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();
    return true;
}

bool ShaderResource::GenerateLoadCommand(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);

    ShaderCodeLoader::ShaderCode code;
    if (!shaderCodeLoader->LoadCode(m_ShaderName[ifindex], code)) {
        assert(false);
        return false;
    }

    auto &q = queue;
    auto &m = Memory;

    auto *arg = q.PushCommand<Commands::ConstructShader>();

    arg->m_Valid.fill(false);
    arg->m_CodeArray.fill("");
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();
    arg->m_ShaderOutput = &m_ShaderHandle[ifindex];

    for (const auto &shadertype : arg->ShaderTypes) {
        auto index = static_cast<unsigned>(shadertype.m_Type);

        if (code.m_Code[index].empty())
            continue;

        std::string buffer;
        buffer.reserve(16 * 1024); //value is not important

        char buf[64];

        buffer += "#version 420\n";
        buffer += "\n";
        buffer += "//defines begin\n";
        buffer += fmt::format("#define {}_main main\n", shadertype.m_Name);
        buffer += fmt::format("#define shader_{}\n", shadertype.m_Name);
        buffer += "#define SHINESS_SCALER 128.0f\n";
        buffer += "\n";
        buffer += GenerateGaussianDisc(shaderConfiguration->gaussianDiscLength, shaderConfiguration->gaussianDiscRadius);
        buffer += "\n";
        buffer += "//preprocessed code begin\n";
        buffer += code.m_Code[index];
        buffer += "//preprocessed code end\n";
        buffer += "\n";

        arg->m_CodeArray[index] = m.CloneString(buffer);

#ifdef DEBUG_DUMP
        std::string sname = m_ShaderName[ifindex];
        std::replace(sname.begin(), sname.end(), '/', '.');
        std::ofstream out(std::string("logs/") + sname + "." + shadertype.m_Name);
        out << buffer;
        out.close();
#endif

        arg->m_Valid[index] = true;
    }
           
    return true;
}

bool ShaderResource::InitializeUniforms(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);
    assert(m_ShaderInterface[ifindex]);

    auto iface = m_ShaderInterface[ifindex];

    auto *arg = q.PushCommand<Commands::GetShaderUnfiorms>();
    arg->m_Count = iface->UniformCount();
    arg->m_Names = iface->UniformName();
    arg->m_ShaderHandle = &m_ShaderHandle[ifindex];
    arg->m_Locations = &m_ShaderUniform[ifindex];
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();
        
    return true;
}

bool ShaderResource::InitializeSamplers(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);
    assert(m_ShaderInterface[ifindex]);

    auto iface = m_ShaderInterface[ifindex];

    auto cnt = iface->SamplerCount();

    if (cnt <= 1)//single sampler, no need to do anything
        return true;

    auto *arg = q.PushCommand<Commands::InitShaderSamplers>();
    arg->m_Count = cnt;
    arg->m_Names = iface->SamplerName();
    arg->m_ShaderHandle = &m_ShaderHandle[ifindex];
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();

    return true;
}

//---------------------------------------------------------------------------------------

std::string ShaderResource::GenerateGaussianDisc(size_t length, float baseRadius) {
    static const size_t seed = 0xFADEDEAD;

    //const float radius = 2.0f; //TODO:?

    std::mt19937 mt(seed);
    std::normal_distribution<> gaussian(1.0f);
    std::uniform_real_distribution<> uniform(0.0f, 1.0f);

    std::string buf;
    buf.reserve(4096);

    length = std::max((size_t)1, length);

#ifdef DEBUG_DUMP
    buf += fmt::format("// baseRadius={}\n", baseRadius);
    buf += fmt::format("// seed=0x{:X}\n", seed);
#endif
    buf += fmt::format("#define GAUSSIAN_DISC_SIZE {}\n", length);
    buf += "uniform vec2 GAUSSIAN_DISC[GAUSSIAN_DISC_SIZE] = {\n";
    buf += fmt::format("\tvec2({}, {}),\n", 0.0f, 0.0f);
    for (size_t i = 1; i < length; ++i) {
        float a = uniform(mt) * 2.0f * emath::constant::pi<float>;
        float r = gaussian(mt) * baseRadius;
        float x = r * sin(a);
        float y = -r * cos(a);
#ifdef DEBUG_DUMP
        buf += fmt::format("// r={} a={}\n", r, a);
#endif
        buf += fmt::format("\tvec2({}, {}),\n", x, y);
    }
    buf += "};\n";
    buf += "\n";

    return buf;
}   

//---------------------------------------------------------------------------------------

void ShaderResource::Dump(Space::OFmtStream &o) {
    for (unsigned index = 0u; index < m_ShaderName.size(); ++index) {
        if (!m_ShaderLoaded[index].load())
            continue;

        o.line("%2u -> %s", index, m_ShaderName[index].c_str());
    }
}

} //namespace MoonGlare::Renderer::Resources::Shader

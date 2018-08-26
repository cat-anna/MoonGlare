/*
  * Generated by cppsrc.sh
  * On 2017-02-22 19:09:53,00
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/


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

bool ShaderResource::Initialize(ResourceManager *Owner, iFileSystem *fileSystem) {
    assert(Owner);
    assert(fileSystem);

    m_ResourceManager = Owner;
    shaderCodeLoader = std::make_unique<Loader>(fileSystem);

    m_ShaderHandle.fill(Device::InvalidShaderHandle);
    m_ShaderInterface.fill(nullptr);
    Conf::UniformLocations locs;
    locs.fill(Device::InvalidShaderUniformHandle);
    m_ShaderUniform.fill(locs);
    m_ShaderName.fill(std::string());
    for(auto &item: m_ShaderLoaded)
        item = false;

    return true;
}

bool ShaderResource::Finalize() {
    return true;
}

//---------------------------------------------------------------------------------------

void ShaderResource::ReloadAll() {
    RendererAssert(this);

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
    RendererAssert(this);
    if (handle.m_TmpGuard != handle.GuardValue) {
        AddLogf(Error, "Invalid handle!");
        return false; 
    }
    return GenerateReload(queue, Memory, handle.m_Index);
}

bool ShaderResource::GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex) {
    RendererAssert(this);
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
    RendererAssert(this);
    RendererAssert(ShaderIface);

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
    RendererAssert(this);
    auto *arg = q.PushCommand<Commands::ReleaseShaderResource>();
    arg->m_ShaderHandle = &m_ShaderHandle[ifindex];
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();
    return true;
}

bool ShaderResource::GenerateLoadCommand(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex) {
    RendererAssert(this);

    ShaderCodeLoader::ShaderCode code;
    if (!shaderCodeLoader->LoadCode(m_ShaderName[ifindex], code)) {
        RendererAssert(false);
        return false;
    }

    auto &q = queue;
    auto &m = Memory;

    auto *arg = q.PushCommand<Commands::ConstructShader>();

    arg->m_Valid.fill(false);
    arg->m_CodeArray;
    arg->m_ShaderName = m_ShaderName[ifindex].c_str();
    arg->m_ShaderOutput = &m_ShaderHandle[ifindex];

    for (const auto &shadertype : arg->ShaderTypes) {
        auto index = static_cast<unsigned>(shadertype.m_Type);

        auto &Lines = arg->m_CodeArray[index];
        Lines.fill("\n");

        if (code.m_Code[index].empty())
            continue;

        char buf[64];

        Lines[0] = "#version 420\n";
        Lines[1] = "//defines begin\n";

        sprintf_s(buf, "#define %s_main main\n", shadertype.m_Name);
        Lines[2] = m.CloneString(buf);

        sprintf_s(buf, "#define shader_%s\n", shadertype.m_Name);
        Lines[3] = m.CloneString(buf);
        //4
        //5
        Lines[6] = "//preprocessed code begin\n";
        Lines[7] = m.CloneString(code.m_Code[index]);

#ifdef DEBUG_DUMP
        std::string sname = m_ShaderName[ifindex];
        std::replace(sname.begin(), sname.end(), '/', '.');
        std::ofstream out(std::string("logs/") + sname + "." + shadertype.m_Name);
        for (auto &l : Lines)
            out << l;
        out.close();
#endif

        arg->m_Valid[index] = true;
    }
           
    return true;
}

bool ShaderResource::InitializeUniforms(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex) {
    RendererAssert(this);
    RendererAssert(m_ShaderInterface[ifindex]);

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
    RendererAssert(this);
    RendererAssert(m_ShaderInterface[ifindex]);

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

void ShaderResource::Dump(Space::OFmtStream &o) {
    for (unsigned index = 0u; index < m_ShaderName.size(); ++index) {
        if (!m_ShaderLoaded[index].load())
            continue;

        o.line("%2u -> %s", index, m_ShaderName[index].c_str());
    }
}

} //namespace MoonGlare::Renderer::Resources::Shader

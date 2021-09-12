#include "shader_resource.hpp"
#include "commands/shader_init_commands.hpp"
#include "debugger_support.hpp"
#include <cassert>
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Resources::Shader {

ShaderResource::ShaderResource(gsl::not_null<iAsyncLoader *> _async_loader,
                               gsl::not_null<iReadOnlyFileSystem *> _file_system,
                               gsl::not_null<iShaderCodeLoader *> _shader_loader,
                               gsl::not_null<iContextResourceLoader *> _context_loader,
                               gsl::not_null<CommandQueue *> init_commands)
    : async_loader(_async_loader), file_system(_file_system), shader_loader(_shader_loader),
      context_loader(_context_loader) {

    for (auto &item : shader_loaded) {
        item = false;
    }
    for (auto &item : shader_allocated) {
        item = false;
    }

    //TODO: shaders are leaking!

    init_commands->PushCommand(Commands::GenShaderBatchCommand{
        .out = standby_shaders_pool.BulkInsert(standby_shaders_pool.Capacity()),
        .count = standby_shaders_pool.Capacity(),
    });

    // shaderConfiguration = &Owner->GetConfiguration()->shader;
    // shaderCodeLoader = std::make_unique<Loader>(fileSystem);
    // m_ShaderHandle.fill(Device::InvalidShaderHandle);
    // m_ShaderInterface.fill(nullptr);
    // Conf::UniformLocations locs;
    // locs.fill(Device::InvalidShaderUniformHandle);
    // m_ShaderUniform.fill(locs);
}

ShaderResource::~ShaderResource() = default;

void ShaderResource::ReloadAllShaders() {
    for (auto i = 0u; i < shader_loaded.size(); ++i) {
        if (shader_loaded[i]) {
            DebugLog(Warning, fmt::format("Reloading shader {}:{}", i, shader_base_name[i]));
            ScheduleReload(i);
        }
    }
}

//---------------------------------------------------------------------------------------

ResourceHandle ShaderResource::LoadShaderResource(FileResourceId resource_id) {
    auto full_name = file_system->GetNameOfResource(resource_id, true);
    if (auto dot_pos = full_name.find_last_of('.'); dot_pos != std::string::npos) {
        full_name.resize(dot_pos);
    }

    return LoadShaderByPath(full_name);
}

ShaderHandle ShaderResource::LoadShader(const std::string &name) {
    if (name.empty()) {
        return 0;
    }
    if (name[0] == '/') {
        return LoadShaderByPath(name);
    } else {
        return LoadShaderByPath("/shader/" + name);
    }
}

ShaderHandle ShaderResource::LoadShaderByPath(const std::string &base_path) {
    for (size_t i = 1; i < shader_base_name.size(); ++i) {
        //TODO: switch to some more fancy search?
        if (shader_base_name[i] == base_path) {
            return i | kResourceTypeShader;
        }
    }

    Device::ShaderHandle device_handle = Device::kInvalidShaderHandle;

    //TODO: check and handle if there is standby shader program

    device_handle = standby_shaders_pool.Next();
    shader_base_name[device_handle] = base_path;
    ScheduleReload(device_handle);

    return device_handle | kResourceTypeShader;

    // auto ifindex = ShaderIface->InterfaceID();

    // bool isalloc = false;
    // if (!m_ShaderLoaded[ifindex].compare_exchange_strong(isalloc, true)) {
    //     out.m_TmpGuard = out.GuardValue;
    //     out.m_Index = static_cast<uint16_t>(ifindex);
    //     return out;
    // }

    // m_ShaderName[ifindex] = ShaderName;
    // m_ShaderHandle[ifindex] = Device::InvalidShaderHandle;
    // m_ShaderUniform[ifindex].fill(Device::InvalidShaderUniformHandle);
    // m_ShaderInterface[ifindex] = ShaderIface;

    // out.m_TmpGuard = out.GuardValue;
    // out.m_Index = static_cast<uint16_t>(ifindex);

    // auto loader = dynamic_cast<AsyncLoader *>(m_ResourceManager->GetLoader()); //TODO: this is ugly
    // loader->SubmitShaderLoad(out);
}

void ShaderResource::ScheduleReload(Device::ShaderHandle device_handle) {
    shader_loaded[device_handle] = false;
    shader_variables[device_handle] = ShaderVariables{};

    shader_loader->ScheduleLoadCode(
        shader_base_name[device_handle], [this, device_handle](iShaderCodeLoader::ShaderCode code) {
            context_loader->PushResourceTask(
                [this, device_handle, shader_code = std::move(code)](auto q) {
                    LoadShaderCode(&q, device_handle, shader_code);
                    ReloadUniforms(&q, device_handle);
                    shader_loaded[device_handle] = true;
                    AddLog(Resources, fmt::format("Loaded shader: source={} device_handle={}",
                                                  shader_base_name[device_handle], device_handle));
                });
        });
}

void ShaderResource::LoadShaderCode(CommandQueue *queue, Device::ShaderHandle device_handle,
                                    const iShaderCodeLoader::ShaderCode &code) const {

    Commands::ConstructShaderCommand command;

    command.Reset();
    command.shader_name = shader_base_name[device_handle].c_str();
    command.handle = device_handle;

    for (const auto &shader_type : iShaderCodeLoader::kShaderFiles) {
        auto index = static_cast<unsigned>(shader_type.shader_type);
        if (code.code[index].empty()) {
            continue;
        }
        command.code_array[index] = queue->GetMemory().CloneString(code.code[index]);
    }

    queue->PushCommand(std::move(command));
}

void ShaderResource::ReloadUniforms(CommandQueue *queue, Device::ShaderHandle device_handle) {
    queue->PushCommand(Commands::QueryStandardUniformsCommand{
        .handle = device_handle,
        .uniforms = &shader_variables[device_handle],
    });
    queue->PushCommand(Commands::InitShaderSamplersCommand{
        .handle = device_handle,
        .shader_name = shader_base_name[device_handle].c_str(),
    });
}

const ShaderVariables *ShaderResource::GetShaderVariables(ShaderHandle handle) {
    auto dev_handle = handle & kResourceDeviceHandleMask;
    if (dev_handle < Configuration::Shader::kLimit &&
        (handle & kResourceTypeMask) == kResourceTypeShader && //
        shader_loaded[dev_handle]) {
        return &shader_variables[dev_handle];
    } else {
        // AddLog(Error, "GetShaderVariables: Invalid shader handle!"); //TODO
        return &ShaderVariables::GetInvalidHandles();
    }
}

#if 0

//---------------------------------------------------------------------------------------

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

//---------------------------------------------------------------------------------------

bool ShaderResource::ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex) {
    assert(this);
    auto *arg = q.PushCommand<Commands::ReleaseShaderResource>();
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

#endif

} //namespace MoonGlare::Renderer::Resources::Shader

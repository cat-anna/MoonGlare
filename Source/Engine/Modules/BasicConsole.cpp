#include <pch.h>

#define NEED_VAO_BUILDER

#include <nfMoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Font.h>
#include "BasicConsole.h"
#include <Engine/Core/Engine.h>
#include <Engine/Core/Configuration.Runtime.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/Frame.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/Texture/TextureResource.h>
#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/Mesh/VAOResource.h>

#include <Source/Renderer/Commands/CommandQueue.h>
#include <Source/Renderer/Commands/OpenGL/ControllCommands.h>
#include <Source/Renderer/Commands/OpenGL/TextureCommands.h>

#include <Memory/EnumMapper.h>

#include <Renderer/VirtualCamera.h>
#include <Core/Scripts/ScriptEngine.h>

#include <Foundation/GLFWKeyMapping.h>

namespace MoonGlare::Modules {

class BasicConsole::ConsoleLine {
public:
    ConsoleLine(float Time, iConsole::LineType type = iConsole::LineType::Regular): lineType(type),
        ShowTime(Time){ };

    ConsoleLine(const ConsoleLine&) = delete;
    ~ConsoleLine() { }

    iConsole::LineType lineType = iConsole::LineType::Regular;
    float ShowTime;
    wstring Text;

    DataClasses::iFont::FontResources m_FontResources{ };

    bool m_Ready = false;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

class BasicConsole::InputLine {
public:
    InputLine(BasicConsole *Owner) : m_Text(), m_CaretPos(0), m_Owner(Owner) {}
    ~InputLine() {}
    const wstring& GetString() const {
        return m_Text;
    }
    void SetString(const wstring &data) {
        m_Text = data;
        m_CaretPos = m_Text.length();
    }
    void Clear() {
        m_Text.clear();
        m_CaretPos = 0;
        m_TextValid = false;
    }
    void PutChar(unsigned key) {
        typedef KeyMapping Key;
        switch ((Key) key) {
        case Key::ArrowLeft:
            if (m_CaretPos < 1) return;
            --m_CaretPos;
            break;
        case Key::ArrowRight:
            if (m_CaretPos >= (int) m_Text.length()) return;
            ++m_CaretPos;
            break;
        case Key::Backspace:
            if (m_CaretPos < 1) return;
            m_Text.erase(m_CaretPos - 1, 1);
            --m_CaretPos;
            break;
        case Key::Delete:
            if (m_CaretPos >= (int) m_Text.length()) return;
            m_Text.erase(m_CaretPos, 1);
            break;
        case Key::Tab:
            AddLog(Debug, "Tab pressed");
            break;
        default:
            if (key > 255) return;
            if (!isprint(key)) return;
            m_Text.insert(m_CaretPos, 1, static_cast<char>(key));
            ++m_CaretPos;
            break;
        }
        m_TextValid = false;
    }
    
    wstring DisplayText() const {
        static const std::wstring Prompt = L"> ";
        wstring text = Prompt + GetString();
        text.insert(m_CaretPos + Prompt.length(), 1, '|');
        return std::move(text);
    }
    
    DataClasses::iFont::FontResources m_FontResources{  };
    bool m_TextValid = false;
protected:
    wstring m_Text;
    int m_CaretPos;
    BasicConsole *m_Owner;
};

//-------------------------------------------------------------------------------------------------
         
struct LineColorTable : public Memory::EnumMapper<iConsole::LineType, emath::fvec4> {
    LineColorTable() {
        using E = iConsole::LineType;
        at(E::Regular)   = emath::fvec4{ 0.8f, 0.8f, 0.8f, 1.0f, };
        at(E::Highlight) = emath::fvec4{ 1.0f, 1.0f, 1.0f, 1.0f, };
        at(E::Error)     = emath::fvec4{ 1.0f, 0.5f, 0.5f, 1.0f, };
        at(E::Warning)   = emath::fvec4{ 0.8f, 0.8f, 0.0f, 1.0f, };
        at(E::Hint)      = emath::fvec4{ 0.6f, 1.0f, 0.6f, 1.0f, };
        at(E::Debug)     = emath::fvec4{ 0.5f, 0.5f, 1.0f, 1.0f, };
    }
};
       
static const LineColorTable LineTypesColor;

//-------------------------------------------------------------------------------------------------

BasicConsole::BasicConsole(InterfaceMap &ifaceMap) :
        interfaceMap(ifaceMap),
        m_Font(nullptr), 
        m_MaxLines(20), 
        m_Lines(), 
        m_InputLine(std::make_unique<InputLine>(this)), 
        m_Flags(0),
        m_Active(false) {

    SetVisible(true);
    SetHideOldLines(true);
}

BasicConsole::~BasicConsole() {
    Clear();
    m_Font.reset();
}

void BasicConsole::PostInit() {
    Core::RuntimeConfiguration *cfg;
    interfaceMap.GetObject(cfg);
    Core::Data::Manager *dm;
    interfaceMap.GetObject(dm);
    m_Font = dm->GetFont(cfg->consoleFont);

    auto &shres = Core::GetEngine()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetShaderResource();
    shres.Load(m_ShaderHandle, "Passthrough");
}

//-------------------------------------------------------------------------------------------------

void BasicConsole::AddLine(const std::string &line, LineType type) {
    return AddLine(Utils::Strings::towstring(line), type);
}

void BasicConsole::Clear() {
    m_Lines.clear();
    m_InputLine->Clear(); 
}

void BasicConsole::Deactivate() {
    m_InputLine->Clear(); 
    m_Active = false;
}

void BasicConsole::Activate() {
    m_InputLine->Clear();
    m_Active = true;
}

bool BasicConsole::ProcessConsole(const Core::MoveConfig &config) {
    if (m_Lines.empty() && !m_Active) {
        //BasicConsole has nothing to process
        return true;
    }

    Renderer::Frame *frame = config.m_BufferFrame;

    using PassthroughShaderDescriptor = Renderer::PassthroughShaderDescriptor;
    using Uniform = PassthroughShaderDescriptor::Uniform;
    using Sampler = PassthroughShaderDescriptor::Sampler;

    auto &shres = frame->GetResourceManager()->GetShaderResource();

    static Renderer::VirtualCamera Camera;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        Camera.SetDefaultOrthogonal(config.m_ScreenSize);
    }

    auto qptr = frame->AllocateSubQueue();
    if (!qptr)
        return false;
    auto &q = *qptr;

    using namespace ::MoonGlare::Renderer;

    auto key = Commands::CommandKey();

    auto shb = shres.GetBuilder(q, m_ShaderHandle);
    shb.Bind();
    shb.Set<Uniform::CameraMatrix>(Camera.GetProjectionMatrix());
    shb.Set<Uniform::BackColor>(emath::fvec4(1,1,1,1));
    q.MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);
    q.MakeCommand<Commands::Enable>((GLenum)GL_BLEND);

    auto PrintText = [this, &q, key, frame, &shb](const emath::fvec3 &position,
                    Renderer::TextureResourceHandle &Texture,
                    Renderer::VAOResourceHandle &VAO) {
        Eigen::Affine3f a{ Eigen::Translation3f(position) };
        shb.Set<Uniform::ModelMatrix>(a.matrix());
        shb.Set<Sampler::DiffuseMap>(Texture, key);

        auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, VAO);
        vaob.BindVAO();

        q.MakeCommand<Commands::VAODrawTriangles>(6u, (unsigned)Device::TypeInfo<uint8_t>::TypeId);
    };
   
    if (!m_Lines.empty()) {
        if ((IsHideOldLines() && glfwGetTime() >= m_Lines.front().ShowTime) || m_Lines.size() > m_MaxLines) {
            auto &line = m_Lines.front();

            line.m_FontResources.Release(frame);
            m_Lines.pop_front();
            while (m_Lines.size() > 5 * m_MaxLines) 
                m_Lines.pop_front();
        }

        static constexpr float LineH = 15.0f;		
        emath::fvec3 position(5, -10, 0);
        
        unsigned idx = 0u;
        for (auto &line : m_Lines) {
            ++idx;
            if (idx > m_MaxLines)
                break;

            position.y() += LineH;

            if (!line.m_Ready) {
                DataClasses::iFont::FontRenderRequest req{ };
                req.m_Color = LineTypesColor[line.lineType];
                req.m_Size = 16;
                DataClasses::iFont::FontRect rect;
                DataClasses::iFont::FontDeviceOptions devop{ false, };

                m_Font->RenderText(line.Text, frame, req, devop, rect, line.m_FontResources);
                line.m_Ready = true;
            }

            PrintText(position, 
                line.m_FontResources.m_Texture, line.m_FontResources.m_VAO);
        }
    }

    if (m_Active) {
        if (!m_InputLine->m_TextValid) {
            DataClasses::iFont::FontRenderRequest req;
            req.m_Color = LineTypesColor[LineType::Regular];
            req.m_Size = 16;
            DataClasses::iFont::FontRect rect;
            DataClasses::iFont::FontDeviceOptions devop{ false, };
            m_InputLine->m_TextValid = m_Font->RenderText(m_InputLine->DisplayText(), frame, req, devop, rect, m_InputLine->m_FontResources);
        }
        if (m_InputLine->m_TextValid) {
            auto pos = emath::fvec3(5, m_MaxLines * 15 + 15 + 5, 0);
            PrintText(pos,
                m_InputLine->m_FontResources.m_Texture, m_InputLine->m_FontResources.m_VAO);
        }
    }

    frame->Submit(qptr, Renderer::Configuration::FrameBuffer::Layer::PostRender);
    return true;
}

void BasicConsole::AddLine(wstring Text, LineType lineType) {
    m_Lines.emplace_back((float)glfwGetTime() + 60, lineType);
    auto &line = m_Lines.back();
    line.Text = std::move(Text);
}

void BasicConsole::AsyncLine(const string &Text, LineType lineType) {
    Core::GetEngine()->PushSynchronizedAction([=]() {
        AddLine(Text, lineType);
    });
}

void BasicConsole::PushKey(unsigned key) {
    using Key = KeyMapping;
    switch ((Key) key) {
    case Key::Enter:
    {
        wstring text = m_InputLine->GetString();
        m_InputLine->Clear();
        if (text.empty())
            return;
        AddLine(text);
        Core::GetScriptEngine()->ExecuteCode(Utils::Strings::tostring(text), "ConsoleInput");
        return;
    }
    default:
        m_InputLine->PutChar(key);
    }
}

} //namespace MoonGlare::Modules

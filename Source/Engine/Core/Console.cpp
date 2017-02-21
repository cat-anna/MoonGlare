#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/DataClasses/iFont.h>
#include "Console.h"
#include <Engine/Core/Engine.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/Frame.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Resources/ResourceManager.h>

#include <Source/Renderer/Commands/CommandQueue.h>
#include <Source/Renderer/Commands/OpenGL/ControllCommands.h>
#include <Source/Renderer/Commands/OpenGL/TextureCommands.h>

#include "Graphic/Shaders/D2Shader.h"

namespace MoonGlare::Core {

class Console::ConsoleLine {
public:
	ConsoleLine(float Time, unsigned Type = (unsigned)OrbitLogger::LogChannels::Info): type(Type), 
		ShowTime(Time){ 
	};
	ConsoleLine(const ConsoleLine&) = delete;
	~ConsoleLine() { }
	unsigned type;
	float ShowTime;
	wstring Text;

	DataClasses::Fonts::iFont::FontResources m_FontResources{ 0 };

	bool m_Ready = false;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

class Console::InputLine {
public:
	InputLine(Console *Owner) : m_Text(), m_CaretPos(0), m_Owner(Owner) {}
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
		m_NeedRefresh = true;
	}
	void PutChar(unsigned key) {
		typedef Graphic::WindowInput::Key Key;
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
		m_NeedRefresh = true;
	}
	
	wstring DisplayText() const {
		static const std::wstring Prompt = L"> ";
		wstring text = Prompt + GetString();
		text.insert(m_CaretPos + Prompt.length(), 1, '|');
		return std::move(text);
	}
	
	DataClasses::Fonts::iFont::FontResources m_FontResources{ 0 };
	bool m_NeedRefresh = false;
	bool m_TextValid = false;
protected:
	wstring m_Text;
	int m_CaretPos;
	Console *m_Owner;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Console);
RegisterApiDerivedClass(Console, &Console::RegisterScriptApi);
RegisterApiInstance(Console, &Console::Instance, "Console");

const emath::fvec4 LineTypesColor[] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f, }, //CC_Line_None
		{ 1.0f, 0.5f, 0.5f, 1.0f, }, //CC_Line_Error
		{ 0.8f, 0.8f, 0.0f, 1.0f, }, //CC_Line_Warning
		{ 0.6f, 1.0f, 0.6f, 1.0f, }, //CC_Line_Hint
		{ 0.8f, 0.8f, 0.8f, 1.0f, }, //Console
};

Console::Console() :
		m_Font(0), 
		m_MaxLines(20), 
		m_Lines(), 
		m_InputLine(std::make_unique<InputLine>(this)), 
		m_Flags(0),
		m_Active(false) {
	SetThisAsInstance();
	SetVisible(true);
	SetHideOldLines(true);
}

Console::~Console() {
}

//-------------------------------------------------------------------------------------------------

bool Console::Initialize() {
	if (!m_Font)
		SetFont(GetDataMgr()->GetConsoleFont());
	return true;
}

bool Console::Finalize() {
	Clear();
	m_Font.reset();
	return true;
}

void Console::RegisterScriptApi(::ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cConsole")
		.addFunction("Clear", &Console::Clear)
		.addFunction("Print", &Console::Print)
		.addFunction("SetVisible", &ThisClass::SetVisible)
	.endClass();
}

//-------------------------------------------------------------------------------------------------

bool Console::SetFont(DataClasses::FontPtr Font) {
	Clear();
	if (!Font) {
		return false;
	}
	m_Font = Font;
	return true;
}

void Console::Clear() {
	m_Lines.clear();
	m_InputLine->Clear(); 
}

void Console::Deactivate() {
	m_InputLine->Clear(); 
	m_Active = false;
}

void Console::Activate() {
	m_InputLine->Clear();
	m_Active = true;
}

bool Console::ProcessConsole(const Core::MoveConfig &config) {
	Renderer::Frame *frame = config.m_BufferFrame;

	if (!m_Shader) {
		if (!Graphic::GetShaderMgr()->GetSpecialShaderType<Graphic::Shaders::Shader>("D2Shader", m_Shader)) {
			AddLogf(Error, "Failed to load D2Shader shader");
		}
	}

	if (!m_Shader)
		return false;

	static Renderer::VirtualCamera Camera;
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		Camera.SetDefaultOrthogonal(math::fvec2(Graphic::GetRenderDevice()->GetContextSize()));
	}

	auto qptr = frame->AllocateSubQueue();
	if (!qptr)
		return false;
	auto &q = *qptr;

	using namespace ::MoonGlare::Renderer;

	auto key = Commands::CommandKey();

	m_Shader->Bind(q, key);
	m_Shader->SetCameraMatrix(q, key, Camera.GetProjectionMatrix());
	q.MakeCommand<Commands::Disable>((GLenum)GL_DEPTH_TEST);

	auto PrintText = [this, &q, key, frame](const emath::fvec3 &position,
					Renderer::TextureResourceHandle &Texture,
					Renderer::VAOResourceHandle &VAO) {
		Eigen::Affine3f a{ Eigen::Translation3f(position) };
		m_Shader->SetModelMatrix(q, key, a.matrix());

		auto texres = q.PushCommand<Commands::Texture2DResourceBind>(key);
		texres->m_Handle = Texture;
		texres->m_HandleArray = frame->GetResourceManager()->GetTextureAllocator().GetHandleArrayBase();

		auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, VAO);
		q.PushCommand<Commands::VAOBindResource>(key)->m_VAO = vaob.m_HandlePtr;

		auto arg = q.MakeCommand<Commands::VAODrawTriangles>(6u, (unsigned)GLTypeInfo<uint8_t>::TypeId);
	};
   
	if (!m_Lines.empty()) {
		if ((IsHideOldLines() && glfwGetTime() >= m_Lines.front().ShowTime) || m_Lines.size() > m_MaxLines) {
			auto &line = m_Lines.front();

			line.m_FontResources.Release(frame);
			m_Lines.pop_front();
		}

		static constexpr float LineH = 15.0f;		
		emath::fvec3 position(5, -10, 0);
		
		unsigned idx = 0u;
		for (auto &line : m_Lines) {
			++idx;
			if (idx >= m_MaxLines)
				break;

			position.y() += LineH;

			if (!line.m_Ready) {
				DataClasses::Fonts::iFont::FontRenderRequest req;
				req.m_Color = LineTypesColor[line.type];
				DataClasses::Fonts::iFont::FontRect rect;
				m_Font->RenderText(line.Text, frame, req, rect, line.m_FontResources);
				line.m_Ready = true;
			}

			PrintText(position, 
				line.m_FontResources.m_Texture, line.m_FontResources.m_VAO);
		}
	}

	if (m_Active) {
		if (m_InputLine->m_NeedRefresh) {
			DataClasses::Fonts::iFont::FontRenderRequest req;
			DataClasses::Fonts::iFont::FontRect rect;
			m_InputLine->m_TextValid = m_Font->RenderText(m_InputLine->DisplayText(), frame, req, rect, m_InputLine->m_FontResources);
		}
		if (m_InputLine->m_TextValid) {
			auto pos = emath::fvec3(5, m_MaxLines * 15 + 15 + 5, 0);
			PrintText(pos,
				m_InputLine->m_FontResources.m_Texture, m_InputLine->m_FontResources.m_VAO);
		}
	}

	frame->Submit(qptr, Renderer::Configuration::Context::Window::First);
	return true;
}

void Console::Print(const char* Text, unsigned lineType) {
	return AddLine(Utils::Strings::towstring(Text), lineType);
}

void Console::AddLine(const string &Text, unsigned lineType) {
	return AddLine(Utils::Strings::towstring(Text), lineType);
}

void Console::AddLine(const wstring &Text, unsigned lineType) {
	m_Lines.emplace_back((float)glfwGetTime() + 60, lineType);
	auto &line = m_Lines.back();
	line.Text = Text;
}

void Console::AsyncLine(const string &Text, unsigned lineType) {
	Core::GetEngine()->PushSynchronizedAction([=]() {
		AddLine(Text, lineType);
	});
}

void Console::PushChar(unsigned key) {
	m_InputLine->PutChar(key);
}

void Console::PushKey(unsigned key) {
	using Key = Graphic::WindowInput::Key;
	switch ((Key) key) {
	case Key::Enter:
	{
		wstring text = m_InputLine->GetString();
		m_InputLine->Clear();
		if (text.empty())
			return;
		AddLine(text);
		GetScriptEngine()->ExecuteCode(Utils::Strings::tostring(text), "ConsoleInput");
		return;
	}
	default:
		m_InputLine->PutChar(key);
	}
}

} //namespace MoonGlare::Core

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
	ConsoleLine(float Time, unsigned Type = (unsigned)OrbitLogger::LogChannels::Info): type(Type), ShowTime(Time), Line() { 
	};
	ConsoleLine(const ConsoleLine&) = delete;
	~ConsoleLine() { }
	unsigned type;
	float ShowTime;
	DataClasses::FontInstance Line;
	wstring Text;

	Renderer::TextureResourceHandle m_TextTexture{0};
	Renderer::VAOResourceHandle m_TextVAO{0};
	bool m_Ready = false;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

class Console::InputLine {
public:
	InputLine(Console *Owner) : m_Text(), m_CaretPos(0), m_Owner(Owner), m_Line() {}
	~InputLine() {}
	DataClasses::FontInstance& GetLine() {
		return m_Line;
	};
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
		if (m_Line)
			m_Line.reset();
		Redraw();
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
		Redraw();
	}
	
	wstring DisplayText() const {
		static const std::wstring Prompt = L"> ";
		wstring text = Prompt + GetString();
		text.insert(m_CaretPos + Prompt.length(), 1, '|');
		return std::move(text);
	}
	
	Renderer::TextureResourceHandle m_TextTexture{ 0 };
	Renderer::VAOResourceHandle m_TextVAO{ 0 };
protected:

	DataClasses::FontInstance m_Line;
	wstring m_Text;
	int m_CaretPos;
	Console *m_Owner;
	void Redraw() {
		m_Line.reset();
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Console);
RegisterApiDerivedClass(Console, &Console::RegisterScriptApi);
RegisterApiInstance(Console, &Console::Instance, "Console");

//const math::fvec3 LineTypesColor[(unsigned)OrbitLogger::LineType::MaxScreenConsole]= {
//		{ 1.0f, 1.0f, 1.0f }, //CC_Line_None
//		{ 1.0f, 0.5f, 0.5f }, //CC_Line_Error
//		{ 0.8f, 0.8f, 0.0f }, //CC_Line_Warning
//		{ 0.8f, 0.8f, 0.8f }, //Console
//		//{ 0.6f, 1.0f, 0.6f }, //CC_Line_Hint
//};

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

bool Console::RenderConsole(Graphic::cRenderDevice &dev) {
	if (!m_Font)
		return false;

	static Renderer::VirtualCamera Camera;
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		Camera.SetDefaultOrthogonal(math::fvec2(Graphic::GetRenderDevice()->GetContextSize()));
	}

	dev.Bind(&Camera);

	if (!m_Lines.empty()) {
		if (IsHideOldLines())
			if (glfwGetTime() >= m_Lines.front().ShowTime)
				m_Lines.pop_front();

		math::mat4 matrix;
		matrix = glm::translate(matrix, math::fvec3(5, -10, 0));
		//m_Font->GetTexture().Bind();
		for (auto it = m_Lines.begin(), jt = m_Lines.end(); it != jt; ++it) {
			//unsigned type = it->type;
			//if (type >= (unsigned)OrbitLogger::LineType::MaxScreenConsole)
			//	type = 0;
			matrix = glm::translate(matrix, math::fvec3(0, 15, 0));
			dev.SetModelMatrix(matrix);
			if (!it->Line) {
				DataClasses::Fonts::Descriptor dummy;
				dummy.Color = math::vec3(1, 1, 1);// LineTypesColor[type];
				it->Line = m_Font->GenerateInstance(it->Text.c_str(), &dummy);
			}
			it->Line->Render(dev);
		}
	}

	if (!m_Active)
		return true;

	auto &input = m_InputLine->GetLine();
	if (input) {
		math::mat4 matrix;
		matrix = glm::translate(matrix, math::fvec3(5, m_MaxLines * 15 + 15 + 5, 0));
		dev.SetModelMatrix(matrix);
		dev.CurrentShader()->SetBackColor(math::vec3(1, 1, 1));// LineTypesColor[(unsigned)OrbitLogger::LogLineType::Normal]);
		input->Render(dev);
	}
	return true;
}

bool Console::ProcessConsole(Renderer::Frame *frame) {
	if (!m_Shader) {
		if (!Graphic::GetShaderMgr()->GetSpecialShaderType<Graphic::Shaders::Shader>("D2Shader", m_Shader)) {
			AddLogf(Error, "Failed to load D2Shader shader");
		}
	}

	if (!m_RtShader) {
		if (!Graphic::GetShaderMgr()->GetSpecialShaderType<Graphic::Shaders::Shader>("rttest", m_RtShader)) {
			AddLogf(Error, "Failed to load RtShader shader");
		}
	}

	if (!m_Shader || !m_RtShader)
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

	auto PrepareTextTexture = [this, frame, key](const std::wstring &text, DataClasses::FontInstance& instance,
								Renderer::TextureResourceHandle &Texture,
								Renderer::VAOResourceHandle &VAO) -> bool{
		DataClasses::Fonts::Descriptor dummy;
		dummy.Color = math::vec3(1, 1, 1);// LineTypesColor[type];
		instance = m_Font->GenerateInstance(text.c_str(), &dummy);

		auto trt = frame->GetDevice()->AllocateTextureRenderTask();
		if (!trt)
			return false;

		auto &fonti = instance;
		auto tsize = m_Font->TextSize(text.c_str(), &dummy, false);

		trt->SetFrame(frame);
		trt->SetTarget(Texture, emath::MathCast<emath::ivec2>(tsize.m_CanvasSize));

		trt->Begin();

		auto &q = trt->GetCommandQueue();

		m_RtShader->Bind(q, key);
		m_RtShader->SetModelMatrix(q, key, emath::MathCast<emath::fmat4>(glm::translate(glm::mat4(), math::vec3(tsize.m_TextPosition, 0))));// emath::MathCast<emath::fmat4>(entry.m_Matrix));

		Renderer::VirtualCamera Camera;
		Camera.SetDefaultOrthogonal(tsize.m_CanvasSize);

		m_RtShader->SetCameraMatrix(q, key, Camera.GetProjectionMatrix());

		//m_RtShader->SetColor(q, key, math::vec4(abs(sin(col)), abs(cos(col)), abs(sin(col)*cos(col)), 1)); //math::vec4(entry.m_FontStyle.Color, 1.0f));
		//m_RtShader->SetTileMode(q, key, math::vec2(0, 0));
		fonti->GenerateCommands(q, 0);

		trt->End();

		auto su = tsize.m_CanvasSize;
		Graphic::QuadArray3 Vertexes{
			Graphic::vec3(0, su[1], 0),
			Graphic::vec3(su[0], su[1], 0),
			Graphic::vec3(su[0], 0, 0),
			Graphic::vec3(0, 0, 0),
		};
		float w1 = 0.0f;
		float h1 = 0.0f;
		float w2 = 1.0f;
		float h2 = 1.0f;
		Graphic::QuadArray2 TexUV{
			Graphic::vec2(w1, h1),
			Graphic::vec2(w2, h1),
			Graphic::vec2(w2, h2),
			Graphic::vec2(w1, h2),
		};

		{
			auto &m = frame->GetMemory();
			using ichannels = Renderer::Configuration::VAO::InputChannels;

			auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, VAO, true);
			vaob.BeginDataChange();

			vaob.CreateChannel(ichannels::Vertex);
			vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Vertexes), Vertexes.size());

			vaob.CreateChannel(ichannels::Texture0);
			vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(TexUV), TexUV.size());

			vaob.CreateChannel(ichannels::Index);
			static constexpr std::array<uint8_t, 6> IndexTable = { 0, 1, 2, 0, 2, 3, };
			vaob.SetIndex(ichannels::Index, IndexTable);

			vaob.EndDataChange();
			vaob.UnBindVAO();
		}

		frame->Submit(trt);
		return true;
	};

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
		if (IsHideOldLines())
			if (glfwGetTime() >= m_Lines.front().ShowTime)
				m_Lines.pop_front();

		static constexpr float LineH = 15.0f;		
		emath::fvec3 position(5, -10, 0);
		
		for (auto &line : m_Lines) {
			position.y() += LineH;

			if (!line.m_Ready) {
				if (PrepareTextTexture(line.Text, line.Line, line.m_TextTexture, line.m_TextVAO))
					line.m_Ready = true;
			}

			PrintText(position, line.m_TextTexture, line.m_TextVAO);
		}
	}

	if (m_Active) {
		auto &input = m_InputLine->GetLine();
		if (input || PrepareTextTexture(m_InputLine->DisplayText(), input, m_InputLine->m_TextTexture, m_InputLine->m_TextVAO)) {
			auto pos = emath::fvec3(5, m_MaxLines * 15 + 15 + 5, 0);
			PrintText(pos, m_InputLine->m_TextTexture, m_InputLine->m_TextVAO);
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
	if (m_Lines.size() >= m_MaxLines)
		m_Lines.pop_front();

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

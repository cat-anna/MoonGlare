#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include <Engine/DataClasses/iFont.h>
#include "Console.h"
#include <Engine/Core/Engine.h>

namespace MoonGlare::Core {

class Console::ConsoleLine {
public:
	ConsoleLine(float Time, unsigned Type = (unsigned)OrbitLogger::LogChannels::Info): type(Type), ShowTime(Time), Line() { };
	ConsoleLine(const ConsoleLine&) = delete;
	~ConsoleLine() { }
	unsigned type;
	float ShowTime;
	DataClasses::FontInstance Line;
	wstring Text;
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
protected:
	wstring m_Text;
	int m_CaretPos;
	DataClasses::FontInstance m_Line;
	Console *m_Owner;
	void Redraw() {
		auto font = m_Owner->GetFont();
		if (!font)
			return;
		static const std::wstring Prompt = L"> ";
		wstring text = Prompt + GetString();
		text.insert(m_CaretPos + Prompt.length(), 1, '|');
		m_Line = font->GenerateInstance(text.c_str(), 0);
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

struct Console::Internals {
	Graphic::VirtualCamera m_Camera;

	Internals() {
		m_Camera.SetDefaultOrthogonal(math::fvec2(Graphic::GetRenderDevice()->GetContextSize()));
	}
};

bool Console::Initialize() {
	if (IsInitialized())
		return true;
	if (!m_Font)
		SetFont(GetDataMgr()->GetConsoleFont());
	m_Internals = std::make_unique<Internals>();
	SetInitialized(true);
	return true;
}

bool Console::Finalize() {
	if (!IsInitialized())
		return true;
	Clear();
	m_Font.reset();
	SetInitialized(false);
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
	if (!IsCanRender() || !m_Font)
		return false;

	dev.Bind(&m_Internals->m_Camera);

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

void Console::Print(const char* Text, unsigned lineType) {
	if (!IsCanRender()) return;
	return AddLine(Utils::Strings::towstring(Text), lineType);
}

void Console::AddLine(const string &Text, unsigned lineType) {
	if (!IsCanRender()) return;
	return AddLine(Utils::Strings::towstring(Text), lineType);
}

void Console::AddLine(const wstring &Text, unsigned lineType) {
	if (!IsCanRender()) return;

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

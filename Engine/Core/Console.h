#ifndef ConsoleH
#define ConsoleH

namespace MoonGlare {
namespace Core {

class Console : public cRootClass {
	GABI_DECLARE_CLASS_SINGLETON(Console, cRootClass)
public:
	class ConsoleLine;
	class InputLine;

	Console();
	~Console();
	bool Initialize();
	bool Finalize();
	void Clear();
	bool SetFont(DataClasses::FontPtr Font);
	void SetMaxLines(unsigned Count) { m_MaxLines = Count; }

	void ProcessInput(unsigned key);
	void CancelInput();

	bool RenderConsole(Graphic::cRenderDevice &dev);

	void AddLine(const string &Text, unsigned lineType = 0);
	void AddLine(const wstring &Text, unsigned lineType = 0);
	void Print(const char* Text, unsigned lineType = 0);

	void AsyncLine(const string &Text, unsigned lineType = 0);

	const DataClasses::FontPtr& GetFont() { return m_Font; }

	enum class Flags {
		Visible,
		HideOldLines,
		Initialized,
	};
	DefineFlag(m_Flags, FlagBit(Flags::Visible), Visible);
	DefineFlag(m_Flags, FlagBit(Flags::HideOldLines), HideOldLines);
	DefineFlagGetter(m_Flags, FlagBit(Flags::Initialized), Initialized);

	DefineFlagGetter(m_Flags, FlagBit(Flags::Initialized) | FlagBit(Flags::Visible) , CanRender);

	static void RegisterScriptApi(::ApiInitializer &api);
protected:
	unsigned m_Flags;
	DataClasses::FontPtr m_Font;
	unsigned m_MaxLines;
	std::list<ConsoleLine> m_Lines;
	std::unique_ptr<InputLine> m_InputLine;
	Graphic::VirtualCameraPtr m_Camera;
	DefineFlagSetter(m_Flags, FlagBit(Flags::Initialized), Initialized);
};

}//namespace Core

inline Core::Console* GetConsole() { return ::MoonGlare::Core::Console::Instance(); }
inline bool ConsoleExists() { return ::MoonGlare::Core::Console::InstanceExists(); }

}//namespace MoonGlare

#endif

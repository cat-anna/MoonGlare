#pragma once

#include <Renderer/PassthroughShaderDescriptor.h>
#include "../iConsole.h"

namespace MoonGlare::Modules {

class BasicConsole : public iConsole {
    using ThisClass = BasicConsole;
public:
    //iConsole
    void PushKey(unsigned key) override;
    void Deactivate() override;
    void Activate() override;
    void Clear() override;
    void AddLine(std::string line, LineType type) override 
    {
    }
    bool ProcessConsole(const Core::MoveConfig &config) override;

    class ConsoleLine;
    class InputLine;

    BasicConsole();
    ~BasicConsole();

    bool Initialize();
    bool Finalize();
    bool SetFont(DataClasses::FontPtr Font);
    void SetMaxLines(unsigned Count) { m_MaxLines = Count; }


    void AddLine(const string &Text, unsigned lineType = 0);
    void AddLine(const wstring &Text, unsigned lineType = 0);
    void Print(const char* Text, unsigned lineType = 0);

    void AsyncLine(const string &Text, unsigned lineType = 0);

    const DataClasses::FontPtr& GetFont() { return m_Font; }

    enum class Flags {
        Visible,
        HideOldLines,
    };
    DefineFlag(m_Flags, FlagBit(Flags::Visible), Visible);
    DefineFlag(m_Flags, FlagBit(Flags::HideOldLines), HideOldLines);

    //static void RegisterScriptApi(::ApiInitializer &api);
protected:
    unsigned m_Flags;
    bool m_Active;
    DataClasses::FontPtr m_Font;
    unsigned m_MaxLines;
    std::list<ConsoleLine> m_Lines;
    std::unique_ptr<InputLine> m_InputLine;
    Renderer::ShaderResourceHandle<Renderer::PassthroughShaderDescriptor> m_ShaderHandle{ };
};

}//namespace MoonGlare::Modules

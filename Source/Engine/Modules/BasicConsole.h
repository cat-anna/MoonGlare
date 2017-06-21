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
    void AddLine(const std::string &line, LineType type) override;
    bool ProcessConsole(const Core::MoveConfig &config) override;

    class ConsoleLine;
    class InputLine;

    BasicConsole();
    ~BasicConsole();

    bool Initialize();
    bool Finalize();
    bool SetFont(DataClasses::FontPtr Font);
    void SetMaxLines(unsigned Count) { m_MaxLines = Count; }

    void AddLine(wstring Text, LineType lineType = LineType::Regular);
    void AsyncLine(const string &Text, LineType lineType = LineType::Regular);

    const DataClasses::FontPtr& GetFont() { return m_Font; }

    enum class Flags {
        Visible,
        HideOldLines,
    };
    DefineFlag(m_Flags, FlagBit(Flags::Visible), Visible);
    DefineFlag(m_Flags, FlagBit(Flags::HideOldLines), HideOldLines);
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

#pragma once

#include "lua_linter.h"
#include <QPlainTextEdit>
#include <QTimer>

namespace MoonGlare::Tools {

class QtLuaHighlighter;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = 0);
    ~CodeEditor() override;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    class LineNumberArea;

    CompilationResult GetCompilationResult() const { return currentLineStatus; }
signals:
    void CodeCompiled();

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    QWidget *lineNumberArea;
    std::unique_ptr<QtLuaHighlighter> highlighter;
    std::unique_ptr<LuaLinter> linter;
    CompilationResult currentLineStatus;
    QTimer updateLintTimer;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
};

} // namespace MoonGlare::Tools

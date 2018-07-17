#pragma once

#include <QPlainTextEdit>
#include <QTimer>

#include "LuaLinter.h"

class QtLuaHighlighter;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = 0);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth(); 

    class LineNumberArea;
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

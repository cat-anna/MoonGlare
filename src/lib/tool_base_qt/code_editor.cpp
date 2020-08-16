
#include "code_editor.h"
#include "qt_lua_highlighter.h"
#include <QPainter>
#include <fmt/format.h>

namespace MoonGlare::Tools {

class CodeEditor::LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) { m_Owner = editor; }

    QSize sizeHint() const override { return QSize(m_Owner->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override { m_Owner->lineNumberAreaPaintEvent(event); }

private:
    CodeEditor *m_Owner;
};

//-----------------------------------------

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {

    highlighter = std::make_unique<QtLuaHighlighter>(document());
    linter = std::make_unique<LuaLinter>();

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    connect(this, &QPlainTextEdit::textChanged, [this]() { updateLintTimer.start(); });

    updateLintTimer.setInterval(1000);
    updateLintTimer.setSingleShot(true);
    connect(&updateLintTimer, &QTimer::timeout, [this]() {
        currentLineStatus = linter->Compile(this->toPlainText().toLocal8Bit().constData());
        repaint();
        if (currentLineStatus.lineInfo.empty())
            lineNumberArea->setToolTip("");
        else {
            auto &e = *currentLineStatus.lineInfo.begin();
            lineNumberArea->setToolTip(fmt::format("{} : {}", e.first, e.second).c_str());
        }
    });

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QFont font;
    font.setFamily("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    QFontMetrics metrics(font);
    setTabStopWidth(4 * metrics.width(' '));
}

CodeEditor::~CodeEditor() {
    QObject::disconnect();
    highlighter.reset();
    linter.reset();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);

    if (currentLineStatus.lineInfo.empty()) {
        painter.fillRect(event->rect(), QColor(0, 255, 0, 64));
    } else {
        painter.fillRect(event->rect(), Qt::lightGray);
    }

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {

            auto it = currentLineStatus.lineInfo.find(blockNumber + 1);
            if (it != currentLineStatus.lineInfo.end()) {
                painter.fillRect(QRect(0, top, lineNumberArea->width(), (int)blockBoundingRect(block).height()),
                                 Qt::red);
            }

            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(3, top, lineNumberArea->width() - 6, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    ++digits;
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits + 6;
    return space;
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount) { setViewportMargins(lineNumberAreaWidth(), 0, 0, 0); }

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

} // namespace MoonGlare::Tools

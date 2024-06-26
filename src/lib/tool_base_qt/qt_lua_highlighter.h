#pragma once

#include <QSyntaxHighlighter>

namespace MoonGlare::Tools {

class QtLuaHighlighter : public QSyntaxHighlighter {
    Q_OBJECT;

public:
    explicit QtLuaHighlighter(QTextDocument *document = 0);
    ~QtLuaHighlighter();

protected:
    void highlightBlock(const QString &text);

    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

} // namespace MoonGlare::Tools

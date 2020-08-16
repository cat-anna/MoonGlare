#pragma once

#include <qstring>
#include <qwidget>
#include <string>

namespace MoonGlare::Tools {

struct UserQuestions {
    bool AskForConfirmation(const char *msg = nullptr);
    void ErrorMessage(const char *message);
    void ErrorMessage(const std::string &message);
    void ReportNotImplemented();

    bool QueryStringInput(const QString &Prompt, QString &Input, int MinChars = 4);
    bool QueryStringInput(const QString &Prompt, std::string &Input, int MinChars = 4);

protected:
    virtual ~UserQuestions() {}
    virtual QWidget *GetParentWidget() { return dynamic_cast<QWidget *>(this); }
    virtual QString GetTitle() const { return "MoonGlare"; }
};

} // namespace MoonGlare::Tools

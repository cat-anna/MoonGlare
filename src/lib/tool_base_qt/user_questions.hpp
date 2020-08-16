#pragma once

#include <QString>
#include <qwidget>
#include <string>

namespace MoonGlare::Tools {

struct UserQuestions {
    bool AskForConfirmation(const char *msg = nullptr) const;
    bool AskToDropChanges(const std::string &file = "") const;

    void ErrorMessage(const char *message) const;
    void ErrorMessage(const std::string &message) const;
    void ReportNotImplemented() const;

    bool QueryStringInput(const QString &Prompt, QString &Input, int MinChars = 4) const;
    bool QueryStringInput(const QString &Prompt, std::string &Input, int MinChars = 4) const;

protected:
    virtual ~UserQuestions() {}
    virtual QWidget *GetParentWidget() const {
        return const_cast<QWidget *>(dynamic_cast<const QWidget *>(this));
    }
    virtual QString GetTitle() const { return "MoonGlare"; }
};

} // namespace MoonGlare::Tools

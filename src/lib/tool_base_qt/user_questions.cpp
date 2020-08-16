#include "user_questions.hpp"
#include <QString>
#include <fmt/format.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

namespace MoonGlare::Tools {

bool UserQuestions::AskToDropChanges(const std::string &file) const {
    if (file.empty()) {
        return AskForConfirmation("There are pending changes. Do you wish to continue?");
    } else {
        return AskForConfirmation(
            fmt::format("{} is modified\nDo you wish to continue?", file).c_str());
    }
}

bool UserQuestions::AskForConfirmation(const char *msg) const {
    return QMessageBox::question(GetParentWidget(), GetTitle(), msg ? msg : "Are you sure?") ==
           QMessageBox::Yes;
}

void UserQuestions::ErrorMessage(const std::string &message) const {
    QMessageBox::critical(GetParentWidget(), GetTitle(), QString::fromStdString(message));
}

void UserQuestions::ErrorMessage(const char *message) const {
    QMessageBox::critical(GetParentWidget(), GetTitle(), message);
}

void UserQuestions::ReportNotImplemented() const {
    QMessageBox::information(GetParentWidget(), GetTitle(), "Not implemented");
}

bool UserQuestions::QueryStringInput(const QString &Prompt, QString &Input, int MinChars) const {
    bool succ = false;
    auto ret = QInputDialog::getText(GetParentWidget(), GetTitle(), Prompt, QLineEdit::Normal,
                                     Input, &succ);
    if (!succ)
        return false;
    if (ret.length() < MinChars) {
        ErrorMessage("Too short input!");
        return false;
    }
    Input.swap(ret);
    return succ;
}

bool UserQuestions::QueryStringInput(const QString &Prompt, std::string &Input,
                                     int MinChars) const {
    QString q_input = Input.c_str();
    if (!QueryStringInput(Prompt, q_input, MinChars))
        return false;
    Input = q_input.toLocal8Bit().constData();
    return true;
}

} // namespace MoonGlare::Tools

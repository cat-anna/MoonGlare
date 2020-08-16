#pragma once

#include "user_questions.hpp"
#include <QString>
#include <qinputdialog.h>
#include <qmessagebox.h>

namespace MoonGlare::Tools {

bool UserQuestions::AskForConfirmation(const char *msg) {
    return QMessageBox::question(GetParentWidget(), GetTitle(), msg ? msg : "Are you sure?") ==
           QMessageBox::Yes;
}

void UserQuestions::ErrorMessage(const std::string &message) {
    QMessageBox::critical(GetParentWidget(), GetTitle(), QString::fromStdString(message));
}

void UserQuestions::ErrorMessage(const char *message) {
    QMessageBox::critical(GetParentWidget(), GetTitle(), message);
}

void UserQuestions::ReportNotImplemented() {
    QMessageBox::information(GetParentWidget(), GetTitle(), "Not implemented");
}

bool UserQuestions::QueryStringInput(const QString &Prompt, QString &Input, int MinChars) {
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

bool UserQuestions::QueryStringInput(const QString &Prompt, std::string &Input, int MinChars) {
    QString qinput = Input.c_str();
    if (!QueryStringInput(Prompt, qinput, MinChars))
        return false;
    Input = qinput.toLocal8Bit().constData();
    return true;
}

} // namespace MoonGlare::Tools

#pragma once

#include <qinputdialog.h>
#include <qmessagebox.h>

namespace MoonGlare {

struct UserQuestions {
	bool AskForPermission(const char *msg = nullptr) {
		return QMessageBox::question(GetParentWidget(), GetTitle(), msg ? msg : "Are you sure?") == QMessageBox::Yes;
	}
	void ErrorMessage(const char *message) {
		QMessageBox::critical(GetParentWidget(), GetTitle(), message);
	}
	void ReportNotImplemented() {
		QMessageBox::information(GetParentWidget(), GetTitle(), "Not implemented");
	}

	bool QuerryStringInput(const QString &Prompt, QString &Input, int MinChars = 4) {
		bool succ = false;
		auto ret = QInputDialog::getText(GetParentWidget(), GetTitle(), Prompt, QLineEdit::Normal, Input, &succ);
		if (!succ)
			return false;
		if (ret.length() < MinChars) {
			ErrorMessage("Too short input!");
			return false;
		}
		Input.swap(ret);
		return succ;
	}

	bool QuerryStringInput(const QString &Prompt, std::string &Input, int MinChars = 4) {
		QString qinput = Input.c_str();
		if (!QuerryStringInput(Prompt, qinput, MinChars))
			return false;
		Input = qinput.toLocal8Bit().constData();
		return true;
	}

protected:
	virtual ~UserQuestions() {}
	virtual QWidget* GetParentWidget() { return dynamic_cast<QWidget*>(this); }
	virtual QString GetTitle() const { return "MoonGlare"; }
};

} 

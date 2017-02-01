/*
  * Generated by cppsrc.sh
  * On 2016-07-21 20:23:39,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Utils_H
#define Utils_H

namespace MoonGlare {
namespace QtShared {

class Utils {
public:
	static const char *GetMoonGlareProjectFilter() {
		return "MoonGlare module project (*.mgp)";
	}

	static bool AskForPermission(QWidget *parent, const char *msg = nullptr) {
		return QMessageBox::question(parent, "MoonGlare", msg ? msg : "Are you sure?") == QMessageBox::Yes;
	}
	static void ReportNotImplemented(QWidget *parent) {
		QMessageBox::information(parent, "MoonGlare", "Not implemented");
	}
private: 
	Utils();
};

struct UserQuestions {

	bool AskForPermission( const char *msg = nullptr) {
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

} //namespace QtShared 
} //namespace MoonGlare 

#endif

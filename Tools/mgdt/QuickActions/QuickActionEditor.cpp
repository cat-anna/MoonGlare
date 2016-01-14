#include PCH_HEADER
#include "mgdtSettings.h"
#include "ui_QuickActionEditor.h"
#include "QuickActionEditor.h"
#include "QtLuaHighlighter.h"

QuickActionEditor::QuickActionEditor(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::QuickActionEditor();
	ui->setupUi(this);

	connect(ui->buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));

	ui->CodeTextEdit->setFont(mgdtSettings::get().Editor.GetEditorFont());
	m_Highlighter = new QtLuaHighlighter(ui->CodeTextEdit->document());
}

QuickActionEditor::~QuickActionEditor() {
	delete m_Highlighter;
	delete ui;
}

void QuickActionEditor::Set(const Action_t &a) {
	ui->NameLineEdit->setText(a.Name.c_str());
	ui->GroupLineEdit->setText(a.Group.c_str());
	ui->CodeTextEdit->setPlainText(a.Script.c_str());
}

void QuickActionEditor::Get(Action_t &a) const {
	a.Name = ui->NameLineEdit->text().toUtf8().constData();
	a.Group = ui->GroupLineEdit->text().toUtf8().constData();
	a.Script = ui->CodeTextEdit->toPlainText().toUtf8().constData();
}

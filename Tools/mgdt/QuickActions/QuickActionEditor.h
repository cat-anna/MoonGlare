#ifndef QuickActionEditor_H
#define QuickActionEditor_H

namespace Ui { class QuickActionEditor; }
class QtLuaHighlighter;

class QuickActionEditor : public QDialog {
	Q_OBJECT
public:
	QuickActionEditor(QWidget *parent = 0);
	~QuickActionEditor();

#ifdef mgdtSettings_H
	using Action_t = mgdtSettings::QuickActions_t::Action_t;
	void Set(const Action_t &a) ;
	void Get(Action_t &a) const;
#endif
protected:
private:
	QtLuaHighlighter *m_Highlighter;
	Ui::QuickActionEditor *ui;
protected slots:
};

#endif // MAINFORM_H

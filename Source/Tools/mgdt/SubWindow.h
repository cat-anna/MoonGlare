#ifndef SUBWINDOW_H
#define SUBWINDOW_H

class SubWindow : public QMainWindow {
	Q_OBJECT
public:
	SubWindow(QWidget *parent);
	~SubWindow();

	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);
signals:
	void WindowClosed();
private:
};

#endif // SUBWINDOW_H

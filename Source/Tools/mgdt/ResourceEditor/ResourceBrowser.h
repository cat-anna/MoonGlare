#ifndef RESOURCEBROWSER_H
#define RESOURCEBROWSER_H

#include <SubWindow.h>

namespace Ui {class ResourceBrowser;};
class ResourceEditorBaseTab;

class ResourceBrowser : public SubWindow
{
	Q_OBJECT
public:
	ResourceBrowser(QWidget *parent = 0);
	~ResourceBrowser();

public slots:
	void Refresh();
protected:
	ResourceEditorBaseTab* CurrentEditor();
private:
	Ui::ResourceBrowser *ui;
};

#endif // RESOURCEBROWSER_H

#ifndef SCRIPTLISTWIDGETT_H
#define SCRIPTLISTWIDGETT_H

#include "ResourceEditorBaseTab.h"

namespace Ui { class AudioListWidget; };

class AudioListWidget : public ResourceEditorBaseTab {
	Q_OBJECT
public:
	AudioListWidget(QWidget *parent = 0);
	~AudioListWidget();
protected:
	class LuaRequest; 
	void ResetTreeView();
private:
	Ui::AudioListWidget *ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
public slots:
	void Refresh() override;
protected slots:
	void PlayAudioAction();
	void StopAudioAction();
	void CopyPathAction();
	void ItemDoubleClicked(const QModelIndex& index);
;
};

#endif // LUAGLOBALSTATEWIDGET_H

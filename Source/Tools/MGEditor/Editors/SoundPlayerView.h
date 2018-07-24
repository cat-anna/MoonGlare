#pragma once

#include <qwidget.h>
#include <iEditor.h>

#include <EngineBase/SoundSystem/iSoundSystem.h>

namespace Ui { class SoundPlayerView; }

namespace MoonGlare::Editor {

class SoundPlayerView
		: public QWidget
        , public QtShared::iEditor
		//, public QtShared::iChangeContainer
		//, public QtShared::iEditor 
{
	Q_OBJECT;
public:
    SoundPlayerView(QWidget *parent, SoundSystem::iSoundSystem *ss);
 	virtual ~SoundPlayerView();

    bool OpenData(const std::string &uri) override;
public slots:
    void Refresh();
protected slots:

private: 
	std::unique_ptr<Ui::SoundPlayerView> ui;
    std::unique_ptr<QTimer> timer;
    SoundSystem::iSoundSystem *soundSystem;
    SoundSystem::HandleApi handleApi;
    SoundSystem::Handle handle = SoundSystem::Handle::Invalid;
};

} 

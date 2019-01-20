#include PCH_HEADER
#include "SoundPlayerView.h"
#include <ui_SoundPlayerView.h>

#include <fmt/format.h>

namespace MoonGlare::Editor {

SoundPlayerView::SoundPlayerView(QWidget * parent, SoundSystem::iSoundSystem *ss)
    :  QWidget(parent), soundSystem(ss), handleApi(ss->GetHandleApi()) {
    ui = std::make_unique<Ui::SoundPlayerView>();
    ui->setupUi(this);

    connect(ui->pushButtonPlay, &QPushButton::clicked, [this]() { handleApi.Play(handle); });
    connect(ui->pushButtonPause, &QPushButton::clicked, [this]() { handleApi.Pause(handle); });
    connect(ui->pushButtonStop, &QPushButton::clicked, [this]() { handleApi.Stop(handle); });

    timer = std::make_unique<QTimer>();
    connect(timer.get(), &QTimer::timeout, this, &SoundPlayerView::Refresh);
    timer->setSingleShot(false);
    timer->setInterval(100);
    timer->start();
}

SoundPlayerView::~SoundPlayerView() {
    handleApi.Close(handle, false);
    handle = SoundSystem::SoundHandle::Invalid;
    timer.reset();
    ui.reset();
}

bool SoundPlayerView::OpenData(const std::string &uri) {
    //sound = soundSystem->OpenSound(uri, true);

    ui->labelURI->setText(uri.c_str());
    handle = handleApi.Open(uri, true, SoundSystem::SoundKind::Music, false);
    if (handle == SoundSystem::SoundHandle::Invalid) {
        //todo
    } else {

    }

    Refresh();
    return true;
}

void SoundPlayerView::Refresh() {
    float duration = handleApi.GetDuration(handle);
    float position = handleApi.GetTimePosition(handle);

    ui->progressBar->setValue(static_cast<int>((position * 100)));
    ui->progressBar->setMaximum(static_cast<int>((duration * 100)) * (position <= duration));

    auto timeToString = [](float v) {
        float secs;
        float msecf = modf(v, &secs);
        int min = static_cast<int>(secs / 60.0f);
        int sec = static_cast<int>(secs) % 60;
        int msec = static_cast<int>(msecf * 10);

        return fmt::format("{0:02}:{1:02}.{2:01}", min, sec, msec);
    };

    std::string text = timeToString(position) + "/" + timeToString(duration) + "  ";
    switch (handleApi.GetState(handle)) {
    case SoundSystem::SoundState::Paused:
        text += "[Paused]";
        break;
    case SoundSystem::SoundState::Playing:
        text += "[Playing]";
        break;
    case SoundSystem::SoundState::Stopped:
        text += "[Stopped]";
        break;
    case SoundSystem::SoundState::Invalid:
    default:
        text += "[Invalid]";
        break;
    }
    ui->labelPosition->setText(text.c_str());
}

} 


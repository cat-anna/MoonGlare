#include PCH_HEADER
#include <ui_SoundPlayerView.h>
#include "SoundPlayerView.h"

#include <fmt/format.h>

namespace MoonGlare::Editor {

SoundPlayerView::SoundPlayerView(QWidget * parent, SoundSystem::iSoundSystem *ss)
    :  QWidget(parent), soundSystem(ss) {
    ui = std::make_unique<Ui::SoundPlayerView>();
    ui->setupUi(this);

    connect(ui->pushButtonPlay, &QPushButton::clicked, [this]() { sound->Play(); });
    connect(ui->pushButtonPause, &QPushButton::clicked, [this]() { sound->Pause(); });
    connect(ui->pushButtonStop, &QPushButton::clicked, [this]() { sound->Stop(); });

    timer = std::make_unique<QTimer>();
    connect(timer.get(), &QTimer::timeout, this, &SoundPlayerView::Refresh);
    timer->setSingleShot(false);
    timer->setInterval(100);
    timer->start();
}

SoundPlayerView::~SoundPlayerView() {
    ui.reset();
}

bool SoundPlayerView::OpenData(const std::string &uri) {
    sound = soundSystem->OpenSound(uri, true);
    Refresh();
    ui->labelURI->setText(uri.c_str());
    return true;
}

void SoundPlayerView::Refresh() {
    float duration = sound->GetDuration();

    float position = sound->GetPosition();

    ui->progressBar->setValue(static_cast<int>((position * 100)));
    ui->progressBar->setMaximum(static_cast<int>((duration * 100)) * (position < duration));

    auto timeToString = [](float v) {
        float secs;
        float msecf = modf(v, &secs);
        int min = static_cast<int>(secs / 60.0f);
        int sec = static_cast<int>(secs) % 60;
        int msec = static_cast<int>(msecf * 10);

        return fmt::format("{0:02}:{1:02}.{2:01}", min, sec, msec);
    };

    std::string text = timeToString(position) + "/" + timeToString(duration);
    ui->labelPosition->setText(text.c_str());
}

} 


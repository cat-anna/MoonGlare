#include "dock_window.hpp"
#include <qevent.h>

namespace MoonGlare::Tools {

DockWindow::DockWindow(QWidget *parent, SharedModuleManager smm)
    : QDockWidget(parent), module_manager(std::move(smm)) {

    connect(this, &DockWindow::UIUpdate, this, &DockWindow::UIUpdateImpl, Qt::QueuedConnection);
    connect(&refresh_timer, &QTimer::timeout, [this] { Refresh(); });
}

DockWindow::~DockWindow() {
}

void DockWindow::UIUpdateImpl(std::function<void()> h) {
    h();
}

//-----------------------------------------

void DockWindow::closeEvent(QCloseEvent *event) {
    event->accept();
    emit WindowClosed(this);
}

void DockWindow::showEvent(QShowEvent *event) {
    event->accept();
}

void DockWindow::DoSaveSettings(nlohmann::json &json) const {
    iWidgetSettingsProvider::DoSaveSettings(json);
}

void DockWindow::DoLoadSettings(const nlohmann::json &json) {
    // isFloating();
    setFloating(false);
    setAllowedAreas(Qt::AllDockWidgetAreas);
    iWidgetSettingsProvider::DoLoadSettings(json);
}

void DockWindow::SetAutoRefresh(bool value, unsigned Interval) {
    if (!value) {
        refresh_timer.stop();
    } else {
        refresh_timer.setInterval(Interval);
        refresh_timer.setSingleShot(false);
        refresh_timer.start();
    }
}

void DockWindow::Refresh() {
}

} // namespace MoonGlare::Tools

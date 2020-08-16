#pragma once

#include "runtime_modules/widget_settings_provider.hpp"
#include "user_questions.hpp"
#include <QDockWidget>
#include <QObject>
#include <QTimer>
#include <memory>
#include <runtime_modules.h>
#include <runtime_modules/widget_settings_provider.hpp>

namespace MoonGlare::Tools {

class DockWindow : public QDockWidget,
                   public UserQuestions,
                   public std::enable_shared_from_this<DockWindow>,
                   public RuntineModules::iWidgetSettingsProvider {
    Q_OBJECT

public:
    DockWindow(QWidget *parent, SharedModuleManager smm);
    ~DockWindow();

    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

    void SetAutoRefresh(bool value, unsigned Interval = 1000);
signals:
    void WindowClosed(DockWindow *Sender);
    void UIUpdate(std::function<void()> h);
public slots:
    virtual void Refresh();
protected slots:
    void UIUpdateImpl(std::function<void()> h);

protected:
    SharedModuleManager GetModuleManager() {
        if (!module_manager) {
            __debugbreak();
            throw std::runtime_error("There is no ModuleManager attached");
        }
        return module_manager;
    }
    void DoSaveSettings(nlohmann::json &json) const override;
    void DoLoadSettings(const nlohmann::json &json) override;

private:
    QTimer refresh_timer;
    SharedModuleManager module_manager;
};

} // namespace MoonGlare::Tools

Q_DECLARE_METATYPE(std::function<void()>);

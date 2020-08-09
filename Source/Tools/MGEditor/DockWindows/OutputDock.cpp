/*
 * Generated by cppsrc.sh
 * On 2016-07-22  0:08:40,44
 * by Paweu
 */
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "OutputDock.h"

#include "OutputDockTab.h"
#include <DockWindowInfo.h>
#include <icons.h>
#include <ui_OutputDock.h>

#include <ToolBase/Module.h>

#include <fmt/format.h>

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

struct OutputProxy : public iOutput {
    std::string title;

    void PushLine(const std::string &line) override {
        {
            LOCK_MUTEX(mutex);
            lines.emplace_back(line);
        }
        if (endpoint) {
            auto guard = endpoint;
            guard->PushLine(line);
        }
    };
    void PushLines(const std::list<std::string> &line) override {
        {
            LOCK_MUTEX(mutex);
            for (auto &i : line)
                lines.emplace_back(i);
        }
        if (endpoint) {
            auto guard = endpoint;
            guard->PushLines(line);
        }
    };

    void SetEndpoint(SharedOutput e) {
        endpoint = e;
        if (e) {
            LOCK_MUTEX(mutex);
            e->PushLines(lines);
        }
    }

protected:
    SharedOutput endpoint;
    std::list<std::string> lines;
    std::recursive_mutex mutex;
};

//----------------------------------------------------------------------------------

struct OutputDockInfo : public QtShared::BaseDockWindowModule, public iOutputProvider {

    std::weak_ptr<OutputDock> dock;

    virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
        auto ptr = std::make_shared<OutputDock>(parent, GetModuleManager());
        dock = ptr;
        return ptr;
    }

    OutputDockInfo(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) {
        SetSettingID("OutputDock");
        SetDisplayName(tr("Output"));
        SetShortcut("F10");
    }

    std::unordered_map<std::string, std::weak_ptr<iOutput>> outputs;
    mutable std::recursive_mutex outputsMutex;

    SharedOutput CreateOutput(const std::string &id, const std::string &title) override {
        LOCK_MUTEX(outputsMutex);
        auto existing = GetOutput(id);
        if (existing)
            return existing;

        auto proxy = std::make_shared<OutputProxy>();
        outputs[id] = proxy;
        proxy->title = title;

        auto sdock = dock.lock();
        if (sdock) {
            sdock->UIUpdate([sdock, proxy, title, id]() { proxy->SetEndpoint(sdock->CreateOutput(title, id)); });
        }

        return proxy;
    };

    SharedOutput GetOutput(const std::string &id) override {
        LOCK_MUTEX(outputsMutex);
        auto it = outputs.find(id);
        if (it != outputs.end()) {
            return it->second.lock();
        }
        return nullptr;
    }
    std::unordered_map<std::string, SharedOutput> GetAllOutputs() const override {
        LOCK_MUTEX(outputsMutex);
        std::unordered_map<std::string, SharedOutput> r;
        for (auto &it : outputs) {
            auto ptr = it.second.lock();
            if (ptr)
                r[it.first].swap(ptr);
        }
        return std::move(r);
    }
    void Close(const std::string &id) override {
        LOCK_MUTEX(outputsMutex);
        auto it = outputs.find(id);
        if (it != outputs.end()) {
            auto proxy = std::dynamic_pointer_cast<OutputProxy>(it->second.lock());
            if (proxy) {
                proxy->SetEndpoint(nullptr);
            }
        }
    }
};
ModuleClassRegister::Register<OutputDockInfo> OutputDockInfoReg("OutputDock");

//----------------------------------------------------------------------------------

OutputDock::OutputDock(QWidget *parent, SharedModuleManager modmgr)
    : QtShared::DockWindow(parent), moduleManager(std::move(modmgr)) {

    SetSettingID("OutputDock");
    m_Ui = std::make_unique<Ui::OutputDock>();
    m_Ui->setupUi(this);
    m_Ui->tabWidget->setTabsClosable(true);

    ReopenOutputs();

    connect(m_Ui->tabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        auto *tab = m_Ui->tabWidget->widget(index);
        auto ptr = dynamic_cast<OutputTabWidget *>(tab);
        if (ptr) {
            auto sptr = ptr->shared_from_this();
            auto it = tabs.find(sptr);
            if (it != tabs.end()) {
                moduleManager->QueryModule<iOutputProvider>()->Close(it->second);
                tabs.erase(it);
            }
        }
    });
}

OutputDock::~OutputDock() {
    Clear();
    m_Ui.reset();
}

void OutputDock::Clear() {
    if (tabs.empty())
        return;

    auto op = moduleManager->QueryModule<iOutputProvider>();
    if (op) {
        for (auto &o : tabs) {
            op->Close(o.second);
        }
    }
    tabs.clear();
}

void OutputDock::closeEvent(QCloseEvent *event) {
    DockWindow::closeEvent(event);
    Clear();
}

//----------------------------------------------------------------------------------

bool OutputDock::DoSaveSettings(pugi::xml_node node) const {
    QtShared::DockWindow::DoSaveSettings(node);
    return true;
}

bool OutputDock::DoLoadSettings(const pugi::xml_node node) {
    QtShared::DockWindow::DoLoadSettings(node);
    return true;
}

//----------------------------------------------------------------------------------

SharedOutput OutputDock::CreateOutput(const std::string &title, const std::string &id) {
    auto output = std::make_shared<OutputTabWidget>(this);
    tabs[output] = id;
    m_Ui->tabWidget->addTab(output.get(), title.c_str());
    return output;
}

void OutputDock::ReopenOutputs() {
    for (auto &output : moduleManager->QueryModule<iOutputProvider>()->GetAllOutputs()) {
        auto proxy = std::dynamic_pointer_cast<OutputProxy>(output.second);
        if (proxy) {
            proxy->SetEndpoint(CreateOutput(proxy->title, output.first));
        }
    }
}

void OutputDock::OnTabDestroy(OutputTabWidget *tab) {
    auto idx = m_Ui->tabWidget->indexOf(tab);
    if (idx >= 0)
        m_Ui->tabWidget->removeTab(idx);
}

} // namespace DockWindows
} // namespace Editor
} // namespace MoonGlare

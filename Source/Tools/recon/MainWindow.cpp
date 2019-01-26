#include <QSyntaxHighlighter>
#include <QTextStream>
#include <qmenu.h>
#include <qobject.h>
#include <qtextdocumentwriter.h>

#include <ToolBase/AppConfig.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
             
//--------------------------------

namespace MoonGlare::Recon {

MoonGlare::ModuleClassRegister::Register<MainWindow> MainWindowReg("MainWindow");

MainWindow::MainWindow(SharedModuleManager modmgr)
    : QMainWindow(nullptr), iModule(std::move(modmgr)) {
    SetSettingID("MainWindow");

    ui = std::make_unique<Ui::MainWindow>();
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, [this] () {
        auto txt = ui->plainTextEdit->toPlainText();
        //ui->lineEdit->setText("");
        Send(txt, true);
            });

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem*item) {
        auto t = item->text();
        t.replace("|", "\n");
        Send(t, false);
            });

    connect(ui->buttonBox, &QDialogButtonBox::clicked, [this] (QAbstractButton *item) {
        close();
            });

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, [this] (const QPoint & point) {
        auto item = ui->listWidget->itemAt(point);
        if (item) {
            QMenu menu;
            auto parts = item->text().split("|", QString::SkipEmptyParts);
            for (auto &p : parts) {
                p.replace("\t", "    ");
                menu.addAction(p)->setEnabled(false);
            }

            menu.addSeparator();
            menu.addAction("Send", [this, item] () {
                Send(item->text().replace("|", "\n"), false);
                           });
            menu.addSeparator();
            menu.addAction("Edit", [this, item] () {
                ui->plainTextEdit->setPlainText(item->text().replace("|", "\n"));
                           });
            menu.addAction("Remove", [this, item] () {
                RemoveAll(item->text());
                Save();
                           });
            menu.exec(ui->listWidget->mapToGlobal(point));
        }
            });

#ifdef DEBUG
    setWindowTitle(windowTitle() + " [DEBUG]");
#endif
}

MainWindow::~MainWindow() {
    Save();
    ui.release();
}

bool MainWindow::Initialize() {
    reconClient = GetModuleManager()->QuerryModule<Tools::RemoteConsole::ReconClient>();

    auto appc = GetModuleManager()->QuerryModule<AppConfig>();
    historyFileName = appc->Get("ConfigPath") + "/ReconHistory.txt";

    try {
        Load();
    }
    catch (...) {
        //ignore
    }

    return true;
}

bool MainWindow::PostInit() { 
    show();
    return true;
}

bool MainWindow::Finalize() {
    reconClient.reset();
    return true;
}

bool MainWindow::DoSaveSettings(pugi::xml_node node) const {
    SaveGeometry(node, this, "Qt:Geometry");
    SaveState(node, this, "Qt:State");
    return true;
}

bool MainWindow::DoLoadSettings(const pugi::xml_node node) {
    LoadGeometry(node, this, "Qt:Geometry");
    LoadState(node, this, "Qt:State");
    return true;
}

void MainWindow::RemoveAll(const QString &text) {
    for (int i = 0; i < ui->listWidget->count();) {
        if (ui->listWidget->item(i)->text() == text) {
            ui->listWidget->takeItem(i);
        }
        else {
            ++i;
        }
    }
}

void MainWindow::Send(const QString &text, bool addToHistory) {
    auto t = text;
    t.replace("\n", "|");
    if (addToHistory) {
        RemoveAll(t);
        ui->listWidget->insertItem(0, t);
        Save();
    }

    std::string txt = text.toUtf8().data();
    reconClient->Send(txt);
    ui->statusbar->showMessage(QString("Send ") + text, 5000);
}

void MainWindow::Save() {

    QStringList SL;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        SL.append(ui->listWidget->item(i)->text());
    }
    SL.sort();

    QFile fOut(historyFileName.c_str());
    if (fOut.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream s(&fOut);
        for (int i = 0; i < SL.size(); ++i) {
            s << SL[i] << '\n';
        }
    }
    else {
        return;
    }
}

void MainWindow::Load() {
    QFile fIn(historyFileName.c_str());
    if (fIn.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream sIn(&fIn);
    }
    else {
        return;

    }
    ui->listWidget->clear();

    while (!fIn.atEnd()) {
        QString t = fIn.readLine().trimmed();
        if (!t.isEmpty())
            ui->listWidget->addItem(t);
    }
}

}

#pragma once

namespace Ui { class MainWindow; }

struct ReconData;

class MainWindow
    : public QMainWindow  {
    Q_OBJECT
public:
    MainWindow(std::shared_ptr<ReconData> recon);
    ~MainWindow();
protected:
    std::unique_ptr<Ui::MainWindow> m_Ui;
    std::shared_ptr<ReconData> recon;

    void Send(const QString &text, bool addToHistory);
    void RemoveAll(const QString &text);

    void Save();
    void Load();

};


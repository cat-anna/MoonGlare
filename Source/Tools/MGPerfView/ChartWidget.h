#pragma once

#include <qwidget.h>

namespace Ui { class ChartWidget; }

namespace MoonGlare::PerfView { 

class ChartWidget
    : public QWidget {
    Q_OBJECT;
public:
    ChartWidget(QWidget *parent = nullptr);
    virtual ~ChartWidget();

    void AddChart(QWidget *widget);
    void ReleaseChart(QWidget *widget);
signals:
    void OnCloseButtonPressed(ChartWidget*);
protected:
protected slots:
private: 
    std::unique_ptr<Ui::ChartWidget> ui;
};                   

} 


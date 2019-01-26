#pragma once

#include <array>
#include <mutex>
#include <vector>

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QChart>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/qchartview.h>
#include <QtCharts/qlineseries.h>
#include <QtCore/QDebug>
#include <QtCore/QRandomGenerator>
#include <QtCore/QTimer>

#include "ToolBase/Tools/PerfViewServer.h"

namespace MoonGlare::PerfView { 
using namespace Tools::PerfView;

class ChartView : public QtCharts::QChartView {
    Q_OBJECT
public:
    ChartView(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = 0 );
    virtual ~ChartView();
    
    void SetTitle(const std::string &title);

    void AddSeries(const std::string &name, SeriesInfo seriesInfo, AxisInfo axes); 
    void AddData(SeriesId seriesId, float x, float y);

    void RefreshAxes();

    void resizeEvent(QResizeEvent *event) override;
signals:
    void InactiveTimeout(ChartView*);
protected:

 private:
    std::unique_ptr<QtCharts::QChart> chart;

    std::unique_ptr<QtCharts::QValueAxis> axisX;
    std::unique_ptr<QtCharts::QValueAxis> axisPrimary;
    std::unique_ptr<QtCharts::QValueAxis> axisSecondary;

    struct SeriesData {
        std::unique_ptr<QtCharts::QLineSeries> series;
        std::unique_ptr<QtCharts::QLineSeries> secondarySeries;
        QtCharts::QValueAxis *customAxis;
    };
    std::unordered_map<SeriesId, SeriesData> series;

    QPen GetPen() const;

    float VisibleDuration = 10;

    float xMax = 1;

    std::chrono::steady_clock::time_point lastDataTimePoint = std::chrono::steady_clock::now();
    size_t lastDataIndex = 0;
    std::vector<std::tuple<SeriesId, float,float>> lastValues;

    QTimer timer;


};

}

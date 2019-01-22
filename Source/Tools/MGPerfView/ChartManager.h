#pragma once

#include <array>
#include <mutex>
#include <vector>

#include <QVBoxLayout>
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

class ChartView;
class ChartWidget;

class ChartManager : public QWidget, protected PerfServer {
    Q_OBJECT
public:
    ChartManager(QWidget *parent, QVBoxLayout *chartParent);
    ~ChartManager();

    void Reset();

    void BeginSession() override;
    void AddChart(const std::string_view chartName, ChartId chartId) override;
    void AddSeries(const std::string_view seriesName, SeriesInfo seriesInfo, AxisInfo axisInfo) override;
    void AddData(SeriesId seriesId, float x, float y) override;

protected slots:
    void InactiveTimeout(ChartView*);
    void OnCloseButtonPressed(ChartWidget*);
private:
    struct ChartWidgetInfo {
        std::unique_ptr<ChartView> chartView;
        std::unique_ptr<ChartWidget> chartWidget;

        ChartWidgetInfo() = default;
        ChartWidgetInfo(const ChartWidgetInfo&) = default;
        ChartWidgetInfo(ChartWidgetInfo&&) = default;

        ChartWidgetInfo& operator =(const ChartWidgetInfo&) = default;
        ChartWidgetInfo& operator =(ChartWidgetInfo&&) = default;

        ~ChartWidgetInfo();
    };

    QVBoxLayout *chartParent;
    std::unordered_map<ChartId, ChartWidgetInfo> charts;
    std::unordered_map<SeriesId, ChartView*> series;
    QTimer timer;

    std::mutex actionListMutex;
    std::vector<std::function<void()>> actionList;

    void HandleTimeout();
};

}

#include "qlayout.h"

#include "ChartManager.h"
#include "ChartView.h"
#include "ChartWidget.h"

namespace MoonGlare::PerfView
{
using namespace Tools::PerfView;

ChartManager::ChartManager(QWidget* parent, QLayout* chartParent)
    : QWidget(parent)
    , chartParent(chartParent) {

    QObject::connect(&timer, &QTimer::timeout, this, &ChartManager::HandleTimeout);
    timer.setInterval(100);
    timer.setSingleShot(false);
    timer.start();
}

ChartManager::~ChartManager() {
    Reset();
}

void ChartManager::Reset() {
    for (auto& item : charts) {
        chartParent->removeWidget(item.second.chartWidget.get());
    }
    series.clear();
    charts.clear();
}

void ChartManager::InactiveTimeout(ChartView *cv) {
    for (auto it = charts.begin(), jt = charts.end(); it != jt; ++it) {
        auto &item = *it;
        if (item.second.chartView.get() == cv) {
            chartParent->removeWidget(item.second.chartWidget.get());
            item.second.chartWidget->setVisible(false);
            //charts.erase(it);
            return;
        }
    }
}

void ChartManager::OnCloseButtonPressed(ChartWidget * cw) {
    for (auto it = charts.begin(), jt = charts.end(); it != jt; ++it) {
        auto &item = *it;
        if (item.second.chartWidget.get() == cw) {
            chartParent->removeWidget(item.second.chartWidget.get());
            item.second.chartWidget->setVisible(false);
            //charts.erase(it);
            return;
        }
    }
}

void ChartManager::BeginSession() {
    auto action = [this] () { Reset(); };

    std::lock_guard<std::mutex> lock(actionListMutex);
    actionList.emplace_back(std::move(action));
}

void ChartManager::AddChart(const std::string_view chartName, ChartId chartId) {
    auto action = [this, name = std::string(chartName), chartId]() {
        auto chartWidget = std::make_unique<ChartWidget>();
        chartParent->addWidget(chartWidget.get());

        auto chart = std::make_unique<ChartView>();
        chart->SetTitle(name);
        chartWidget->AddChart(chart.get());

        connect(chart.get(), &ChartView::InactiveTimeout, this, &ChartManager::InactiveTimeout, Qt::QueuedConnection);
        connect(chartWidget.get(), &ChartWidget::OnCloseButtonPressed, this, &ChartManager::OnCloseButtonPressed, Qt::QueuedConnection);

        ChartWidgetInfo cwi;
        cwi.chartView = std::move(chart);
        cwi.chartWidget = std::move(chartWidget);
        charts[chartId] = std::move(cwi);
    };

    std::lock_guard<std::mutex> lock(actionListMutex);
    actionList.emplace_back(std::move(action));
}

void ChartManager::AddSeries(const std::string_view seriesName, SeriesInfo seriesInfo, AxisInfo axisInfo) {
    auto action = [this, name = std::string(seriesName), seriesInfo, axisInfo]() {

        auto &chart = charts[seriesInfo.chartId];
        if (!chart.chartView) {
            __debugbreak();
            return; 
        }

        chart.chartView->AddSeries(name, seriesInfo, axisInfo);
        series[seriesInfo.seriesId] = chart.chartView.get();
    };
    std::lock_guard<std::mutex> lock(actionListMutex);
    actionList.emplace_back(std::move(action));
}

void ChartManager::AddData(SeriesId seriesId, float x, float y) {
    auto action = [this, seriesId, x, y] () {
        auto chart = series[seriesId];
        if (!chart) {
            __debugbreak();
            return;
        }

        chart->AddData(seriesId, x, y);
    };
    std::lock_guard<std::mutex> lock(actionListMutex);
    actionList.emplace_back(std::move(action));
}

void ChartManager::HandleTimeout() {

    decltype(actionList) list;
    list.reserve(1024);
    {
        std::lock_guard<std::mutex> lock(actionListMutex);
        list.swap(actionList);
    }

    for (auto &itm : list) {
        itm();
    }

    for (auto &c : charts) {
        c.second.chartView->RefreshAxes();
    }
}

}


#include "ChartView.h"

#include <Foundation/TimeUtils.h>

namespace MoonGlare::PerfView {

class Chart : public QtCharts::QChart {
public:
    Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):QChart(QtCharts::QChart::ChartTypeCartesian, parent, wFlags){ }
};

ChartView::ChartView(QGraphicsItem *parent, Qt::WindowFlags wFlags) :
    QChartView(new Chart(parent, wFlags))
{
    lastValues.resize(2048);

    setMinimumSize({ 320,240 });

    chart.reset(QChartView::chart());

    //chart->legend()->hide();
    chart->setAnimationOptions(Chart::NoAnimation);

    axisX = std::make_unique<QtCharts::QValueAxis>();
    axisX->setRange(0, VisibleDuration);
    axisX->setTickCount(10);

    axisPrimary = std::make_unique<QtCharts::QValueAxis>();
    axisPrimary->setRange(0, 1);
    //axisSecondary = std::make_unique<QtCharts::QValueAxis>();
    //axisSecondary->setRange(0, 1);

    chart->addAxis(axisX.get(), Qt::AlignBottom);
    chart->addAxis(axisPrimary.get(), Qt::AlignLeft);
    //chart->addAxis(axisSecondary.get(), Qt::AlignRight);

    QObject::connect(&timer, &QTimer::timeout, [this] () {
        if (TimeDiff(lastDataTimePoint, std::chrono::steady_clock::now()) > 30) {
            lastDataTimePoint = std::chrono::steady_clock::now();
            InactiveTimeout(this);
        }
    });
    timer.setInterval(1000);
    timer.setSingleShot(false);
    timer.start();
}

ChartView::~ChartView() {
    series.clear();
    axisX.reset();
    axisPrimary.reset();
    //axisSecondary.reset();
    chart.reset();
}

void ChartView::resizeEvent(QResizeEvent *event) {
    QChartView::resizeEvent(event);
    auto area = chart->plotArea();
    VisibleDuration = area.width()/20.0f;
}

void ChartView::SetTitle(const std::string &title) {
    chart->setTitle(title.c_str());
}

QPen ChartView::GetPen() const {
    QPen pen(Qt::red);
    pen.setWidth(1);

    using namespace Qt;
    static const std::array<QColor, 16> colorArray = {
        black,
        red,
        green,
        blue,
        cyan,
        magenta,
        yellow,
        darkRed,
        darkGreen,
        darkBlue,
        darkCyan,
        darkMagenta,
        darkYellow,
        gray,
        darkGray,
        lightGray,
    };

    pen.setColor(colorArray[series.size() % colorArray.size()]);

    return pen;
}

void ChartView::AddSeries(const std::string &name, SeriesInfo seriesInfo, AxisInfo axes) {
    auto newSeries = std::make_unique<QtCharts::QLineSeries>(chart.get());

    newSeries->setPen(GetPen());
    chart->addSeries(newSeries.get());
    newSeries->attachAxis(axisX.get());
    newSeries->setName(name.c_str());

    newSeries->attachAxis(axisPrimary.get());

    SeriesData si;
    si.series = std::move(newSeries);
    series[seriesInfo.seriesId] = std::move(si);
}

void ChartView::AddData(SeriesId seriesId, float x, float y) {
    auto &s = series[seriesId];
    if (!s.series){
        __debugbreak();
        return;
    }

    lastDataTimePoint = std::chrono::steady_clock::now();

    //while(s.series->count() > 1024)
        //s.series->remove(0);

    s.series->append(x, y);

    y *= 1.05f;
    lastValues[lastDataIndex % lastValues.size()] = { seriesId, x, y };
    ++lastDataIndex;

    xMax = std::max(xMax, x);
}

void ChartView::RefreshAxes() {
    axisX->setRange(std::max(xMax - VisibleDuration, 0.0f), xMax);
    float yMax = 0.01;
    for (auto [sid, lx, ly] : lastValues) {
        if(lx + VisibleDuration > xMax)
            yMax = std::max(yMax, ly);
    }
    axisPrimary->setRange(0, yMax);
    //axisY->setRange(0, yMax);
}

}

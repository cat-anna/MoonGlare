#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <TimeUtils.h>
#include <api/perf_view.h>
#include <settings.h>

namespace MoonGlare::Tools::PerfView {

using Api::AxisInfo;
using Api::ChartId;
using Api::MessageType;
using Api::SeriesId;
using Api::SeriesInfo;
using Api::Unit;

struct PerfServer {
    PerfServer();
    virtual ~PerfServer();

    virtual void BeginSession();
    virtual void AddChart(const std::string_view chartName, ChartId chartId);
    virtual void AddSeries(const std::string_view seriesName, SeriesInfo seriesInfo, AxisInfo axisInfo);
    virtual void AddData(SeriesId seriesId, float x, float y);

private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

} // namespace MoonGlare::Tools::PerfView

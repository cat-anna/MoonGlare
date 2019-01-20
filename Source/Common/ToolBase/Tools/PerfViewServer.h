#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <Foundation/Settings.h>
#include <Foundation/TimeUtils.h>
#include <Foundation/Tools/PerfViewApi.h>

namespace MoonGlare::Tools::PerfView {

using Api::SeriesId;
using Api::ChartId;
using Api::Unit;
using Api::MessageType;
using Api::SeriesInfo;
using Api::AxisInfo;

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

}

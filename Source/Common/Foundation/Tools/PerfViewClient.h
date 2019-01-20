#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <Libs/libSpace/src/Memory/DynamicMessageBuffer.h>

#include <Foundation/InterfaceMap.h>
#include <Foundation/Settings.h>
#include <Foundation/TimeUtils.h>
#include <Foundation/Tools/PerfViewApi.h>

namespace MoonGlare::Tools::PerfView {

#if 1
// def DEBUG_PERF

using Api::SeriesId;
using Api::ChartId;
using Api::Unit;
using Api::MessageType;

struct PerfViewClient {

    PerfViewClient(InterfaceMap &ifaceMap);
    ~PerfViewClient();

    Api::SeriesId AddSeries(const std::string& name, Api::Unit yUnit, Api::Unit xUnit, ChartId chartId);
    void AddData(Api::SeriesId sid, Api::SingleData x, Api::SingleData y) const;
    ChartId AddChart(const std::string &name);

    Api::SingleData GetTimeAxisValue() const
    {
        return static_cast<float>(TimeDiff(startPoint, std::chrono::steady_clock::now()));
    }

private:
    struct Internals;
    std::unique_ptr<Internals> internals;
    std::unordered_map<std::string, Api::SeriesId> seriesMapping;
    std::unordered_map<std::string, ChartId> chartMapping;
    const std::chrono::steady_clock::time_point startPoint = std::chrono::steady_clock::now();
};

struct PerfProducer {
    using Unit = Api::Unit;
    using SeriesId = Api::SeriesId;
    using ChartId = Api::ChartId;

    PerfProducer(InterfaceMap& ifaceMap)
    {
        ifaceMap.GetObject(perfClient);
    }

    ChartId AddChart(const std::string &name) {
        return perfClient->AddChart(name);
    }

    SeriesId AddSeries(const std::string& name, Unit yUnit, ChartId chartId)
    {
        auto sid = perfClient->AddSeries(name, Unit::Miliseconds, yUnit, chartId);
        seriesMapping[name] = sid;
        localSeriesId.push_back(sid);
        return (SeriesId)localSeriesId.size();
    }

    void AddSeries(unsigned sid, const std::string& name, Unit yUnit, ChartId chartId)
    {
        auto extsid = perfClient->AddSeries(name, Unit::Miliseconds, yUnit, chartId);
        seriesMapping[name] = extsid;
        localSeriesId.resize(std::max((size_t)sid + 1, localSeriesId.size() + 1));
        localSeriesId[(SeriesId)sid] = extsid;
    }
    //template <typename T>
    //void AddData(const std::string& name, T value) const
    //{
    //    return AddData(FindSeries(name), GetTimeAxisValue(), value);
    //}

    void AddData(SeriesId sid, float value) const
    {
        AddData(localSeriesId[sid], GetTimeAxisValue(), value);
    }

    Api::SingleData GetTimeAxisValue() const
    {
        return perfClient->GetTimeAxisValue();
    }

private:
    std::shared_ptr<PerfViewClient> perfClient;

    std::unordered_map<std::string, SeriesId> seriesMapping;
    std::vector<SeriesId> localSeriesId = { 0 };

    SeriesId FindSeries(const std::string& name) const
    {
        auto it = seriesMapping.find(name);
        if (it != seriesMapping.end())
            return it->second;
        return 0;
    }

    void AddData(SeriesId sid, Api::SingleData x, Api::SingleData y) const
    {
        perfClient->AddData(sid, x, y);
    }
};

#else

#endif

} // namespace MoonGlare::Tools::PerfView

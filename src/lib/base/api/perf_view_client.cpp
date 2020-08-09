#include "perf_view_client.h"
#include "udp_socket.h"

namespace MoonGlare::Tools::PerfView {

struct PerfViewClient::Internals {

    UdpSocket<Api::MessageBuffer> socket;

    Internals() { socket.ConnectTo(Api::PerfViewPort, "localhost"); }

    void SendPacket(const Api::PayLoad_AddData &data) { socket.Send(data, MessageType::AddData); }
    void SendPacket(const Api::PayLoad_AddSeries &data) { socket.Send(data, MessageType::AddSeries); }
    void SendPacket(const Api::PayLoad_BeginSession &data) { socket.Send(data, MessageType::BeginSession); }
    void SendPacket(const Api::PayLoad_AddChart &data) { socket.Send(data, MessageType::AddChart); }
};

PerfViewClient::PerfViewClient(InterfaceMap &ifaceMap) {
    internals = std::make_unique<Internals>();
    internals->SendPacket(Api::PayLoad_BeginSession{});
}

PerfViewClient::~PerfViewClient() {}

ChartId PerfViewClient::AddChart(const std::string &name) {
    auto it = chartMapping.find(name);
    if (it != chartMapping.end())
        return it->second;

    ChartId cid = static_cast<ChartId>(chartMapping.size() + 1);
    chartMapping[name] = cid;
    Api::PayLoad_AddChart chart = {};
    chart.chartId = cid;
    strncpy_s(chart.name, name.c_str(), name.size());
    internals->SendPacket(chart);
    return cid;
}

SeriesId PerfViewClient::AddSeries(const std::string &name, Unit yUnit, Unit xUnit, ChartId chartId) {
    SeriesId sid = static_cast<SeriesId>(seriesMapping.size() + 1);
    seriesMapping[std::to_string(chartId) + "." + name] = sid;

    Api::PayLoad_AddSeries series = {};
    series.seriesInfo = {chartId, sid};
    series.axes = {xUnit, yUnit};
    strncpy_s(series.name, name.c_str(), name.size());
    internals->SendPacket(series);
    return sid;
}

void PerfViewClient::AddData(Api::SeriesId sid, Api::SingleData x, Api::SingleData y) const {
    if (sid == 0)
        __debugbreak();
    Api::PayLoad_AddData data;
    data.seriesId = sid;
    data.x = x;
    data.y = y;
    internals->SendPacket(data);
}
} // namespace MoonGlare::Tools::PerfView

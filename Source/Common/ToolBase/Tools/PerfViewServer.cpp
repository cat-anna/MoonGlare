
#include "PerfViewServer.h"
#include <Foundation/Tools/UdpSocket.h>

namespace MoonGlare::Tools::PerfView {

struct PerfServer::Internals : public UdpSocket<Api::MessageBuffer> {
    PerfServer* owner;

    Internals(PerfServer* owner)
        : UdpSocket()
        , owner(owner)
    {
        Bind(Api::PerfViewPort);
        Listen();
    }

    void OnMessage(const MessageHeader* request, const udp::endpoint& sender) override
    {
        switch (request->messageType) {
        case MessageType::AddData:
            return AddData((const Api::PayLoad_AddData*)request->payLoad);
        case MessageType::AddSeries:
            return AddSeries((const Api::PayLoad_AddSeries*)request->payLoad);
        case MessageType::AddChart:
            return AddChart((const Api::PayLoad_AddChart*)request->payLoad);
        case MessageType::BeginSession:
            return BeginSession((const Api::PayLoad_BeginSession*)request->payLoad);
        default:
            __debugbreak();
            //todo
        }
    }

    void BeginSession(const Api::PayLoad_BeginSession* data)
    {
        owner->BeginSession();
    }
    void AddChart(const Api::PayLoad_AddChart* data)
    {
        owner->AddChart(data->name, data->chartId);
    }
    void AddSeries(const Api::PayLoad_AddSeries* data)
    {
        owner->AddSeries(data->name, data->seriesInfo, data->axes);
    }
    void AddData(const Api::PayLoad_AddData* data)
    {
        owner->AddData(data->seriesId, data->x, data->y);
    }
};

PerfServer::PerfServer()
{
    internals = std::make_unique<Internals>(this);
}

PerfServer::~PerfServer()
{
}

void PerfServer::BeginSession() {}
void PerfServer::AddChart(const std::string_view chartName, ChartId chartId) {}
void PerfServer::AddSeries(const std::string_view seriesName, SeriesInfo seriesInfo, AxisInfo axisInfo) { }
void PerfServer::AddData(SeriesId seriesId, float x, float y) {}

}

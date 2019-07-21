#pragma once

#include "ApiCommon.h"

namespace MoonGlare::Tools::PerfView::Api {

using namespace Tools::Api;

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma pack(push, 1)

constexpr u16 PerfViewPort = 0xD002;

constexpr Signature SignatureValue = static_cast<Signature>('PERF');

enum class MessageType : u32 {
    NOP = 0,

    BeginSession,
    BeginSessionResponse,

    AddChart,
    AddChartResponse,

    AddSeries,
    AddSeriesResponse,

    AddData,
    AddDataResponse,
};

using MessageHeader = BaseMessageHeader<MessageType, SignatureValue>;
using MessageBuffer = Memory::DynamicMessageBuffer<MaxMessageSize, MessageHeader>;

using SeriesId = u16;
using ChartId = u16;
using SingleData = float;

enum class Unit : u16 {
    Unknown,
    Count,
    Miliseconds,
};

struct SeriesInfo {
    ChartId chartId;
    SeriesId seriesId;
};

struct AxisInfo {
    Unit xUnit;
    Unit yUnit;
};

constexpr size_t StringLength = 256;

struct PayLoad_BeginSession {
};

struct PayLoad_AddChart {
    char name[StringLength];
    ChartId chartId;
};

struct PayLoad_AddSeries {
    char name[StringLength];
    SeriesInfo seriesInfo;
    AxisInfo axes;
};

struct PayLoad_AddData {
    SeriesId seriesId;
    SingleData x, y;
    u32 __padding;
};

#pragma pack(pop)
#pragma warning(pop)
}

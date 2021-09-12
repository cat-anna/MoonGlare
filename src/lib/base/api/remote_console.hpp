#pragma once

#include "api_common.hpp"
#include <cstdint>

namespace MoonGlare::Api::RemoteConsole {

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma pack(push, 1)

constexpr uint16_t kReconPort = 0xD001;
constexpr Signature kSignatureValue = static_cast<Signature>('RCON');

enum class ReconMessageType : uint32_t {
    NOP = 0,

    ExecuteCode,
    ExecuteCodeResponse,

    LogLine,
    LogLineResponse,
};

using MessageHeader = BaseMessageHeader<ReconMessageType, kSignatureValue>;
using MessageBuffer = DynamicMessageBuffer<MaxMessageSize, MessageHeader>;

//----------------------------------------------------------------------------------

struct PayLoad_ExecuteCode {
    char Code[0];
};

struct PayLoad_ExecutionResult {
    int ReturnCode;
};

struct PayLoad_LogLine {
    char Line[0];
};

#pragma pack(pop)
#pragma warning(pop)

} // namespace MoonGlare::Api::RemoteConsole

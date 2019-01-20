#pragma once

#include "ApiCommon.h"

namespace MoonGlare::Tools::RemoteConsole::Api {

using namespace Tools::Api;

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma pack(push, 1)

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

constexpr u16 ReconPort = 0xD001;
constexpr Signature SignatureValue = static_cast<Signature>('RCON');

enum class MessageType : u32 {
    NOP = 0,

    ExecuteCode,
    ExecuteCodeResponse,

    LogLine,
    LogLineResponse,
};

using MessageHeader = BaseMessageHeader<MessageType, SignatureValue>;
using MessageBuffer = Space::Memory::DynamicMessageBuffer<MaxMessageSize, MessageHeader>;

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

} //namespace

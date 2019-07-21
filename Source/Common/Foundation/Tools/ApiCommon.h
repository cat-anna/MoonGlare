#pragma once

#include <Memory/DynamicMessageBuffer.h>

namespace MoonGlare::Tools {
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
}

namespace MoonGlare::Tools::Api {

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma pack(push, 1)

constexpr u32 MaxMessageSize = 32 * 1024;

enum class Signature : u32 {
    Invalid = 0,
};

enum class MessageType : u32 {};

template <typename MessageType_t, Signature SignatureValue_v>
struct BaseMessageHeader {
    using MessageType = MessageType_t;
    static constexpr Signature SignatureValue = SignatureValue_v;

    Signature signature;
    MessageType messageType;
    u32 payloadSize;
    u32 requestID;
    u8 payLoad[0];
};

#pragma pack(pop)
#pragma warning(pop)
}

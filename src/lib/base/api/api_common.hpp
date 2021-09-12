#pragma once

#include "dynamic_message_buffer.hpp"

namespace MoonGlare::Api {

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma pack(push, 1)

constexpr uint32_t MaxMessageSize = 32 * 1024;

enum class Signature : uint32_t {
    Invalid = 0,
};

template <typename MessageType_t, Signature SignatureValue_v>
struct BaseMessageHeader {
    using MessageType = MessageType_t;
    static constexpr Signature kSignatureValue = SignatureValue_v;

    Signature signature;
    MessageType messageType;
    uint32_t payloadSize;
    uint32_t requestID;
    uint8_t payLoad[0];
};

#pragma pack(pop)
#pragma warning(pop)

} // namespace MoonGlare::Api

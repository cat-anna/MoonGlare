#pragma once

#include <atomic>
#include <chrono>
#include <dynamic_message_buffer.hpp>
#include <memory>
#include <optional>
#include <string>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

namespace MoonGlare::Tools {

template <typename BufferType>
struct UdpSocket {

    using udp = boost::asio::ip::udp;

    virtual ~UdpSocket() {
        canRun = false;
        if (thread.joinable())
            thread.join();
    }

    UdpSocket() : io_service(), sock(io_service) {}

    void Bind(uint16_t port) { sock = udp::socket(io_service, udp::endpoint(udp::v4(), port)); }

    void ConnectTo(uint16_t port, const std::string &host) {
        udp::resolver resolver(io_service);
        endpoint = *resolver.resolve({udp::v4(), host, std::to_string(port)});
        sock.open(udp::v4());
    }

    void Listen() {
        if (!thread.joinable()) {
            thread = std::thread([this]() {
                canRun = true;
                ThreadMain();
            });
        }
    }

    using MessageHeader = typename BufferType::Header;
    using MessageType = typename MessageHeader::MessageType;
    constexpr static size_t MaxMessageSize = typename BufferType::Size;

    virtual void OnMessage(const MessageHeader *request, const udp::endpoint &sender) {}

    bool Send(const MessageHeader *header,
              std::optional<udp::endpoint> destination = std::nullopt) {
        uint32_t size = sizeof(MessageHeader) + header->payloadSize;
        sock.send_to(boost::asio::buffer(header, size), destination.value_or(endpoint));
        ++packetsSend;
        bytesSend += size;
        return true;
    }

    template <typename PayloadType>
    void Send(const PayloadType &payload, MessageType messageType, uint32_t requestID = 0,
              std::optional<udp::endpoint> destination = std::nullopt) {
        char buffer[Api::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader *>(buffer);
        header->messageType = messageType;
        header->payloadSize = sizeof(payload) + 1;
        header->requestID = requestID;
        header->signature = MessageHeader::SignatureValue;

        memcpy(header->payLoad, &payload, sizeof(payload));

        Send(header, destination.value_or(endpoint));
    }

private:
    boost::asio::io_service io_service;
    std::thread thread;
    udp::endpoint endpoint;
    udp::socket sock;
    uint64_t packetsSend = 0;
    uint64_t bytesSend = 0;
    std::atomic<bool> canRun = false;

    void ThreadMain() {
        //::OrbitLogger::ThreadInfo::SetName("RECO");

        char buffer[Tools::Api::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader *>(buffer);

        while (canRun) {
            try {
                if (sock.available() <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                sock.receive_from(boost::asio::buffer(buffer, sizeof(buffer) - 1), remote_endpoint,
                                  0, error);

                if (error && error != boost::asio::error::message_size)
                    continue;

                OnMessage(header, remote_endpoint);
            } catch (...) {
                __debugbreak();
            }
        }
    }
};

} // namespace MoonGlare::Tools

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include <Libs/libSpace/src/Memory/DynamicMessageBuffer.h>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

namespace MoonGlare::Tools {

template <typename BufferType>
struct UdpSocket {

    using udp = boost::asio::ip::udp;

    virtual ~UdpSocket()
    {
        canRun = false;
        thread.join();
    }
    
    UdpSocket(uint16_t Port)
        : io_service()
        , sock(io_service, udp::endpoint(udp::v4(), Port))
    {
    }

    UdpSocket(const std::string& Port, const std::string& Host)
        : io_service()
        , sock(io_service)
    {
        udp::resolver resolver(io_service);
        endpoint = *resolver.resolve({ udp::v4(), Host, Port });
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

    virtual void OnMessage(const MessageHeader *request, const udp::endpoint &sender) {}

    bool Send(MessageHeader* header)
    {
        uint32_t size = sizeof(MessageHeader) + header->payloadSize;
        sock.send_to(boost::asio::buffer(header, size), endpoint);
        ++packetsSend;
        bytesSend += size;
        return true;
    }

    template <typename PayloadType>
    void Send(const PayloadType& payload, MessageType messageType, u32 requestID = 0)
    {
        char buffer[Api::MaxMessageSize];
        auto* header = reinterpret_cast<MessageHeader*>(buffer);
        header->messageType = messageType;
        header->payloadSize = sizeof(payload) + 1;
        header->requestID = requestID;
        header->signature = MessageHeader::SignatureValue;

        memcpy(header->payLoad, &payload, sizeof(payload));

        Send(header);
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
        auto *header = reinterpret_cast<MessageHeader*>(buffer);

        while (canRun) {
            try {
                if (sock.available() <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                sock.receive_from(boost::asio::buffer(buffer, sizeof(buffer) - 1), remote_endpoint, 0, error);

                if (error && error != boost::asio::error::message_size)
                    continue;

                OnMessage(header, remote_endpoint);
            }
            catch (...) {
                __debugbreak();
            }
        }
    }
};

} // namespace MoonGlare::Tools::PerfView

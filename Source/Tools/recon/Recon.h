#pragma once

#include <Libs/OrbitLogger/src/OrbitLogger.h>

#include <Foundation/Memory/nMemory.h>
#include <Foundation/Memory/Memory.h>

#include <libSpace/src/Memory/Handle.h>
#include <Engine/Configuration.h>

#include <Foundation/MoonGlareInsider.h>
#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace MoonGlare::Debug::InsiderApi;

struct ReconData {
    ReconData(const std::string &Host, const std::string &Port) : io_service(), s(io_service) {
        udp::resolver resolver(io_service);
        endpoint = *resolver.resolve({ udp::v4(), Host, Port });
        s.open(udp::v4());
    }

    bool Send(MessageHeader *header) {
        s.send_to(boost::asio::buffer(header, sizeof(MessageHeader) + header->PayloadSize), endpoint);
        return true;
    }

    boost::asio::io_service io_service;
    udp::endpoint endpoint;
    udp::socket s;

    void Send(const std::string &txt) {
        char buffer[Configuration::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader*>(buffer);
        header->MessageType = MessageTypes::ExecuteCode;
        char *strbase = (char*)header->PayLoad;

        auto l = std::min(txt.length(), Configuration::MaxMessageSize - sizeof(MessageHeader) - 1);
        strncpy(strbase, txt.c_str(), l);
        strbase[l] = '\0';

        header->PayloadSize = l + 1;

        Send(header);
    }
};
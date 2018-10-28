#include PCH_HEADER

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string.h>
#include <iostream>
#include <memory>
#include <mutex>

#pragma warning ( disable: 4966 )

#include <LibSpace/src/Utils/ParamParser.h>
#include <OrbitLogger/src/OrbitLogger.h>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

#include <Foundation/Memory/nMemory.h>
#include <Foundation/Memory/Memory.h>

#include <libSpace/src/Memory/Handle.h>
#include <Engine/Configuration.h>
#include <Foundation/MoonGlareInsider.h>

#include <libSpace/src/Utils/ParamParser.cpp>

#include "Recon.h"
#include "MainWindow.h"


using namespace std;
using namespace MoonGlare::Debug::InsiderApi;

string _infile = "";
string _Port = std::to_string(Configuration::recon_Port);
string _Host = "localhost";
std::vector<std::string> Lines4Send;

struct Flags {
    enum {
        Buffer		= 1,
        SendFile     = 2,
        Lines    	= 4,
        console = 8,
    };
};

const Space::ProgramParameters::Parameter Parameters[] = {
    {'l', 1, Flags::Lines, Lines4Send, "Send line(s) and exit", 0 },
    {'b', 0, Flags::Buffer, 0, "Buffer whole stdin before send", 0},
    {'f', 1, Flags::SendFile, _infile, "Send content of file and exit", 0 },
    {'c', 0, Flags::console, 0, "do not execute gui", 0 },
    {'p', 1, 0, _Port, "Set port", 0},
    {'h', 1, 0, _Host, "Set host", 0},
    {'\0', 0, 0, 0, 0, 0},
}; 

Space::ProgramParameters Params = {
    Space::ProgramParameters::disable_helpmsg_on_noparams | Space::ProgramParameters::disable_h_as_help,
    "MoonGlare remote console client",
    0,
    0,
    Parameters,
    0,
    0,
}; 


int main(int argc, char** argv) {
    cout << "MoonGlare engine remote console "/* << reconToolVersion.VersionStringFull() <<*/ "\n\n";
    try {
        Params.Parse(argc, argv);
        auto recon = std::make_shared<ReconData>(_Host, _Port);

        char buffer[Configuration::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader*>(buffer);
        header->MessageType = MessageTypes::ExecuteCode;

        char *strbase = (char*)header->PayLoad;

        if (Params.Flags & Flags::SendFile) {
            std::ifstream inp(_infile, std::ios::in | std::ios::binary);
            inp.seekg(0, std::ios::end);
            size_t len = (size_t)inp.tellg();
            inp.seekg(0);
            inp.read(strbase, len);
            strbase[len] = 0;
            header->PayloadSize = strlen(strbase) + 1;
            recon->Send(header);
            return 0;
        }

        if (Params.Flags & Flags::Lines) {
            for (auto &l : Lines4Send) {
                header->PayloadSize = l.length() + 1;
                memcpy(strbase, l.c_str(), l.length());
                strbase[l.length()] = 0;
                recon->Send(header);
            }
            return 0;
        }

        if ((Params.Flags & Flags::console) == 0) {
            cout << "Starting gui...\n";
            int r;
            QApplication a(argc, argv);

            MainWindow w(recon);
            w.show();
            return a.exec();
        }

        bool repetitive = (Params.Flags & Flags::Buffer) == 0;

        do {
            char *strout = strbase;
            *strout = 0;
            while (!std::cin.eof()) {
                string line;
                getline(cin, line);
                strcat(strout, line.c_str());
                strout += line.size();
                if (!(Params.Flags & Flags::Buffer))
                    break;
            }
            header->PayloadSize = strlen(strbase) + 1;
            recon->Send(header);
        } while (repetitive);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    catch (...) {
        return -1;
    }

    return 0;
}

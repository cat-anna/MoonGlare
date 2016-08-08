
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

#include <Version.h>
//#include <reconVersion.inc>
#include <source/Utils/SetGet.h>
#include <source/Utils/Memory/nMemory.h>
#include <source/Utils/Memory.h>

#include <libSpace/src/Memory/Handle.h>
#include <Engine/Configuration.h>
#include <Shared/MoonGlareInsider/Api.h>

using namespace std;
using namespace MoonGlare::Debug::InsiderApi;

string _infile = "";
string _Port = std::to_string(Configuration::recon_Port);
string _Host = "localhost";

struct Flags {
	enum {
		Buffer		= 1,
		SendFile	= 2,
	};
};

const Space::ProgramParameters::Parameter Parameters[] = {
 	{'b', 0, Flags::Buffer, 0, "Buffer whole stdin before send", 0},
	{'f', 0, Flags::SendFile, _infile, "Send content of file and exit", 0 },
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

using boost::asio::ip::udp;

struct ReconData {
	ReconData(const std::string &Host, const std::string &Port) : io_service(), s(io_service) {
		udp::resolver resolver(io_service);
		endpoint = *resolver.resolve({ udp::v4(), Host, Port });
	}

	bool Send(MessageHeader *header) {
		s.send_to(boost::asio::buffer(header, sizeof(MessageHeader) + header->PayloadSize), endpoint);
		return true;
	}

	boost::asio::io_service io_service;
	udp::endpoint endpoint;
	udp::socket s;

};

int main(int argc, char** argv) {
	cout << "MoonGlare engine remote console "/* << reconToolVersion.VersionStringFull() <<*/ "\n\n";
	try {
		Params.Parse(argc, argv);
		ReconData recon(_Host, _Port);

		char buffer[Configuration::MaxMessageSize];
		auto *header = reinterpret_cast<MessageHeader*>(buffer);
		header->MessageType = MessageTypes::ExecuteCode;

		char *strbase = (char*)header->PayLoad;

		if (Params.Flags & Flags::SendFile) {
			printf("not implemented!\n");
			return 0;
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
			recon.Send(header);
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

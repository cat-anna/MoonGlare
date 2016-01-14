
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string.h>
#include <iostream>

#pragma warning ( disable: 4966 )

#include <GabiLib/src/GabiLib.h>
#include <GabiLib/src/utils/ParamParser.h>
#include <GabiLib/src/include_source.hpp>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

#include <Version.h>
#include <reconVersion.inc>
#include <source/Utils/SetGet.h>
#include <source/modules/MoonGlareInsider/MoonGlareInisderApi.h>

using namespace std;
using namespace MoonGlare::Debug::InsiderApi;

string _Port = "";
string _Host = "localhost";

struct Flags {
	enum {
		Buffer		= 1,
	};
};

const GabiLib::ProgramParameters::Parameter Parameters[] = {
 	{'b', 0, Flags::Buffer, 0, "Buffer whole stdin before send", 0},
 	{'p', 1, 0, _Port, "Set port", 0},
 	{'h', 1, 0, _Host, "Set host", 0},
	{'\0', 0, 0, 0, 0, 0},
}; 

GabiLib::ProgramParameters Params = {
	GabiLib::ProgramParameters::disable_helpmsg_on_noparams | GabiLib::ProgramParameters::disable_h_as_help,
	"MoonGlare remote console client",
	0,
	0,
	Parameters,
	0,
	0,
}; 

using boost::asio::ip::udp;

int main(int argc, char** argv) {
	cout << "Moonglare engine remote console " << reconToolVersion.VersionStringFull() << "\n\n";
	{
		char buf[32];
		sprintf(buf, "%d", Configuration::recon_Port);
		_Port = buf;
	}
	try {
		Params.Parse(argc, argv);

		boost::asio::io_service io_service;
		udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

		udp::resolver resolver(io_service);
		udp::endpoint endpoint = *resolver.resolve({udp::v4(), _Host, _Port});

		char buffer[Configuration::MaxMessageSize];
		auto *header = reinterpret_cast<MessageHeader*>(buffer);
		header->MessageType = MessageTypes::ExecuteCode;

		char *strbase = (char*)header->PayLoad;
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

			s.send_to(boost::asio::buffer(buffer, sizeof(MessageHeader) + header->PayloadSize), endpoint);
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

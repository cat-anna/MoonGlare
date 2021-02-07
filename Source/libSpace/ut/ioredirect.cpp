
#include <thread>
#include <string>

#ifdef _MSC_VER

#include <Windows.h>
#include <fcntl.h>
#include <io.h>

struct StdOutCatcher {
	StdOutCatcher() {
		if (IsDebuggerPresent()) {
			std::thread([]() { CatchStdOut(); }).detach();
			std::thread([]() { CatchStdErr(); }).detach();
		}
	}
#define READ_FD 0
#define WRITE_FD 1

#define CHECK(a) if ((a)!= 0) {  return; }

	static void ReadPipe(int pipe) {
		std::string buffer;
		buffer.reserve(512);
		while (true) {
			char buf[64];
			int got = _read(pipe, buf, sizeof(buf));

			if (got <= 0)
				continue;

			for (int i = 0; i < got; ++i) {
				char c = buf[i];
				switch (c) {
				case '\n':
					buffer += c;
					OutputDebugStringA(buffer.c_str());

			//		::Log::LogEngine::Line(Type, 0, Name, Name, buffer.c_str());
					buffer.clear();
				case '\r':
					continue;
				default:
					buffer += c;
				}
			}
		}
	}

	static void CatchStdOut() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		int fdStdOutPipe[2];
		int fdStdOut;

		if (_pipe(fdStdOutPipe, 1024 * 2, O_TEXT) != 0) {
			//treat error eventually
			return;
		}
		fdStdOut = _dup(_fileno(stdout));

		fflush(stdout);

		CHECK(_dup2(fdStdOutPipe[WRITE_FD], _fileno(stdout)));
		std::ios::sync_with_stdio();
		setvbuf(stdout, NULL, _IONBF, 0); // absolutely needed

		ReadPipe(fdStdOutPipe[READ_FD]);
	}
	static void CatchStdErr() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		int fdStdOutPipe[2];
		int fdStdOut;

		if (_pipe(fdStdOutPipe, 1024 * 2, O_TEXT) != 0) {
			//treat error eventually
			return;
		}
		fdStdOut = _dup(_fileno(stderr));

		fflush(stderr);

		CHECK(_dup2(fdStdOutPipe[WRITE_FD], _fileno(stderr)));
		std::ios::sync_with_stdio();
		setvbuf(stderr, NULL, _IONBF, 0); // absolutely needed

		ReadPipe(fdStdOutPipe[READ_FD]);
	}
};

static StdOutCatcher _catcher;

#endif


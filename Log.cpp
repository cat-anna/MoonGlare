/*
 * cLog.cpp
 *
 *  Created on: 14-11-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>
#include <iomanip>

namespace Log {

struct LineTypeVector {
	LineTypeVector() {
		for (unsigned i = 0; i < (unsigned)LogLineType::MaxValue; ++i)
			m_table[i] = " ? ";

#define _at(NAME) m_table[(unsigned)LogLineType:: NAME]
		_at(Error) = "ERR ";
		_at(Warning) = "WARN";
		_at(Console) = "CONS";
		_at(Hint) = "HINT";

		_at(Tool) = "TOOL";
		_at(Debug) = "DBG ";
		_at(DebugWarn) = "DBGW";
		_at(SysInfo) = "SYS ";
		_at(Performance) = "PERF";
		_at(Static) = "STIC";
		
#undef _at
	}
	const char *operator[](LogLineType type) const { return (*this)[(unsigned)type]; }
	const char *operator[](unsigned type) const { return m_table[type]; }
private:
	const char *m_table[(unsigned)LogLineType::MaxValue];
};

LineTypeVector LineType;

//----------------------------------------------------------------------------------

LogSinkBase *_LogSinkList = nullptr;
static std::mutex _LogSinkListMutex;

struct LogEngine::LogEngineImpl {
	using LogLinePool = Utils::Memory::StaticMemoryPool<LogLine, 256, Utils::Memory::NoLockPolicy>;
	using LogLineStringPool = Utils::Memory::StaticStringPool<char, LogLinePool::Size * 256, Utils::Memory::NoLockPolicy>;

	struct LineBuffer {
		std::mutex m_Mutex;
		LogLinePool m_Lines;
		LogLineStringPool m_Strings;
		void Clear() {
			m_Lines.Clear();
			m_Strings.Clear();
		}
	};

	LineBuffer m_Buffers[2];
	LineBuffer *m_Primary, *m_Secondary;
	volatile bool m_ThreadCanRun;
	volatile bool m_ThreadRunning;
	std::chrono::steady_clock::time_point m_ExecutionTime;

	LogEngineImpl() {
		m_ExecutionTime = std::chrono::steady_clock::now();
		m_Primary = m_Buffers;
		m_Secondary = m_Buffers + 1;
		m_Primary->Clear();
		m_Secondary->Clear();

		m_ThreadCanRun = true;
		m_ThreadRunning = false;
		std::thread(&LogEngineImpl::EngineMain, this).detach();
	}

	~LogEngineImpl() {
		m_ThreadCanRun = false;
		while (m_ThreadRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void EngineMain() {
		m_ThreadRunning = true;
		//give some time to settle things
	//	std::this_thread::sleep_for(std::chrono::milliseconds(10));

		SetThisThreadName("LOGE");
		AddLog(Thread, "Log thread executed");
		//EnableScriptsInThisThread(); causes fake memory leak error and log engine does not use scripts

//#if defined(DEBUG) && defined(_BUILDING_ENGINE_)
		//StaticLogCatcher::DispatchLog();
//#endif

		while (true) {
			//I consider this switch as thread-safe
			//the writters will just start to use another pointer
			auto *pool = m_Primary;
			m_Primary = m_Secondary;
			m_Secondary = pool;

			//wait for all writters to finish their job
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			{
				LOCK_MUTEX(_LogSinkListMutex);
				for (auto &it : pool->m_Lines) {
					LogSinkBase *s = _LogSinkList;
					for (; s; s = s->GetPreviousSink()) {
						try {
							s->line(&it);
						}
						catch (...) {
						}
					}
				}
			}

			pool->Clear();

			if (!m_ThreadCanRun && m_Primary->m_Lines.Empty())
				break;

			//std::mutex mutex;
			//std::unique_lock<std::mutex> lock(mutex);
			//_LogThread.wait(lock);
		}

		m_ThreadRunning = false;
	}

	inline void PushLine(LogLineType level, unsigned line, const char *file, const char *fnc, const char* message) {
		auto t = std::chrono::steady_clock::now();
		std::chrono::duration<double> sec = t - _Instance->m_Impl->m_ExecutionTime;

		auto ptr = _Instance->m_Impl->m_Primary;
		const char *mesg;
		LogLine *logline;
		{
			auto *buf = _Instance->m_Impl->m_Primary;
			LOCK_MUTEX(buf->m_Mutex);
			mesg = ptr->m_Strings.Allocate(message);
			logline = ptr->m_Lines.Allocate();
		}

		if (!logline || !mesg)
			return;

		logline->m_Mode = level;
		logline->m_ModeStr = LineType[level];
		logline->m_Message = mesg;
		logline->m_File = file;
		logline->m_Function = fnc;
		logline->m_Line = line;
		logline->m_ExecutionSecs = static_cast<float>(sec.count());
		logline->m_ThreadSign = Thread::GetSignature();
		logline->m_ThreadID = Thread::GetID();
	}
};

//----------------------------------------------------------------------------------

LogEngine* LogEngine::_Instance = nullptr;

LogEngine::LogEngine() {
	_Instance = this;

	m_Impl = std::make_unique<LogEngineImpl>();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

LogEngine::~LogEngine() {
}

void LogEngine::DeleteInstance() {
	delete _Instance;
	_Instance = nullptr;
}

#if 0
void LogEngine::ProcessLine(LogLine *line) {
	def _BUILDING_ENGINE_
	if (ConsoleExists() && line->m_Mode < LogLineType::MaxScreenConsole)
		switch (line->m_Mode) {
		case LogLineType::Console:
			break;
		default: {
			std::stringstream ss;
			ss << LogHeader << line->m_Message;
			string text = ss.str();
			auto mode = line->m_Mode;

			::Core::GetEngine()->PushSynchronizedAction([text, mode]() {
				GetConsole()->AddLine(text, (unsigned)mode);
			});
		}
	}
}
#endif

void LogEngine::Line(LogLineType level, unsigned line, const char *file, const char *fnc, const char* fmt, ...) {
	if (!_Instance)
		return;

	char buffer[4096*2];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	_Instance->m_Impl->PushLine(level, line, file, fnc, buffer);
}

void LogEngine::Line(LogLineType level, unsigned line, const char *file, const char *fnc, std::ostringstream &ss) {
	if (!_Instance)
		return;

	string s = ss.str();
	_Instance->m_Impl->PushLine(level, line, file, fnc, s.c_str());
}

//----------------------------------------------------------------------------------

#if defined(DEBUG) && defined(_BUILDING_ENGINE_)

std::list<StaticLogCatcher*>* StaticLogCatcher::_list = 0;

void StaticLogCatcher::DispatchLog() {
	if (!_list) {
		return;
	}

	_list->sort([](const StaticLogCatcher *c1, const StaticLogCatcher *c2) {
		return static_cast<int>(c2->GetType()) - static_cast<int>(c1->GetType()) > 0;
	});

	while (!_list->empty()) {
		_list->front()->DispatchLine();
		delete _list->front();
		_list->pop_front();
	}

	delete _list;
	_list = nullptr;
}

#endif

//----------------------------------------------------------------------------------

LogSinkBase::LogSinkBase(): m_Previous(nullptr) {
}

LogSinkBase::~LogSinkBase() {
	//Disable();
}

void LogSinkBase::line(const LogLine *line) {
	//Disable();
}

void LogSinkBase::Enable() {
	LOCK_MUTEX(_LogSinkListMutex);
	m_Previous = _LogSinkList;
	_LogSinkList = this;
}

void LogSinkBase::Disable() {
	LOCK_MUTEX(_LogSinkListMutex);
	if (_LogSinkList == this) {
		_LogSinkList = m_Previous;
		m_Previous = nullptr;
		return;
	}

	LogSinkBase *prv = nullptr;
	LogSinkBase *next = _LogSinkList;
	for (; next; prv = next, next = next->m_Previous) {
		if (next != this)
			continue;
		prv->m_Previous = next->m_Previous;
		m_Previous = nullptr;
		break;
	}
}

//----------------------------------------------------------------------------------

void LogStandardBannerWritter::open(const char*file, bool append) {
	LogFileOutputPolicy::open(file, append);

	write(nullptr, "\n======================== SESSION =======================\n");
	write(nullptr, "Engine version: ");
	write(nullptr, ::Core::Engine::GetVersionString().c_str());
	write(nullptr, "\nStart date: ");

	std::time_t t = std::time(NULL);
	char mbstr[100];
	if (std::strftime(mbstr, sizeof(mbstr), "%A %c", std::localtime(&t))) {
		write(nullptr, mbstr);
	} else
		write(nullptr, "{unable to covert date}");
	write(nullptr, "\n\n");
}

LogStandardFormatter::LogStandardFormatter() {
#ifdef _FEATURE_LOG_COUNTERS_
	m_Line = 0;
	memset(m_Type, 0, sizeof(m_Type));
#endif
}

void LogStandardFormatter::format(const LogLine *line, char* buffer, size_t buffer_size) {
	char LogHeader[128];

	char thname[sizeof(Thread::Signature) + 1];
	*((unsigned __int32*)thname) = line->m_ThreadSign;
	thname[sizeof(Thread::Signature)] = 0;
#ifdef _FEATURE_LOG_COUNTERS_
	sprintf(LogHeader, "[%4s:%4d][%7.3f:%4d][%4s:%04x]", 
			line->m_ModeStr, NextType(line->m_Mode),
			line->m_ExecutionSecs, NextLine(),
			thname, line->m_ThreadID);
#else
	sprintf(LogHeader, "[%4s][%7.3f][%4s:%04x]", 
			LineType[line->m_Mode], line->m_ExecutionSecs, thname, line->m_ThreadID);
#endif

	char LogLocation[1024];
	if (line->m_File && line->m_Function) {
		const char *f = strrchr(line->m_File, '\\');
		if(f) ++f;
		else {
			f = strrchr(line->m_File, '/');
			if(f) ++f;
			else f = line->m_File;
		}

		if (line->m_Line)
			sprintf(LogLocation, "[%s:%d@%s] ", f, line->m_Line, line->m_Function);
		else
			sprintf(LogLocation, "[%s] ", f);
	} else
		LogLocation[0] = 0;

	sprintf_s(buffer, buffer_size, "%s%s%s\n", LogHeader, LogLocation, line->m_Message);
}

} // namespace Log

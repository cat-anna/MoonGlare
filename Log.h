/*
 * cLog.h
 *
 *  Created on: 14-11-2013
 *      Author: Paweu
 */

#ifndef CLOG_H_
#define CLOG_H_

namespace Log {

enum class LogLineType {
	None,
	Error,
	Warning,
	Console,
	MaxScreenConsole,

//only log file modes
	Tool,
	Hint,
	Debug,
	DebugWarn,
	SysInfo,
	Performance,
	Static,

	MaxValue,
};

struct LogLine {
	LogLineType m_Mode;
	const char *m_ModeStr;
	const char *m_Message;
	const char *m_File;
	const char *m_Function;
	unsigned m_Line;
	//std::chrono::steady_clock::time_point m_CreationTime;
	float m_ExecutionSecs;
	Thread::Signature m_ThreadSign;
	Thread::NumericID m_ThreadID;

	//LogLine(LogLineType mode = LogLineType::None, unsigned line = 0, const char *file = 0, const char *fnc = 0):
			//m_File(file), m_Line(line), m_Function(fnc), m_Mode(mode) { 
		//m_CreationTime = std::chrono::steady_clock::now();
		//m_ThreadID = Thread::GetID();
		//m_ThreadSign = Thread::GetSignature();
	//}
	//~LogLine() { }
	//void SetMessage(std::ostringstream& ss) { m_Message = ss.str(); }
	//void SetMessage(string s) { m_Message.swap(s); }
	//void SetMode(LogLineType mode) { m_Mode = mode ; }
	//void Queue();
};

struct LogLineProxy {
	template <class T>
	LogLineProxy& operator << (T && t) {
		m_ss << t;
		return *this;
	}

	LogLineProxy(LogLine *line): m_Line(line) { }
	LogLineProxy(const LogLineProxy&) = delete;
	LogLineProxy(LogLineProxy && p) {
		m_ss.swap(p.m_ss);
		m_Line = p.m_Line;
		p.m_Line = nullptr;
	}

	~LogLineProxy() {
		if (!m_Line) return;
		//m_Line->SetMessage(m_ss);
		//m_Line->Queue();
	}

	void SetMode(LogLineType mode) { /*if(m_Line) m_Line->SetMode(mode); */}
protected:
	std::ostringstream m_ss;
	LogLine *m_Line;
};

//--------------------------------------------------------------
//--------------------------------------------------------------

struct LogSinkBase {
	LogSinkBase();
	virtual ~LogSinkBase();
	virtual void line(const LogLine *line) = 0;

	void Enable();
	void Disable();
	LogSinkBase* GetPreviousSink() { return m_Previous; }
private:
	LogSinkBase *m_Previous;
};

template<class OutputPolicy, class FilteringPolicy, class FormatPolicy, size_t BUFFER_SIZE = 4096*2>
struct LogSink : public LogSinkBase, public OutputPolicy, public FilteringPolicy, public FormatPolicy {
	virtual void line(const LogLine *line) override {
		if (!filter(line))
			return;
		char buffer[BUFFER_SIZE];
		format(line, buffer, BUFFER_SIZE);
		write(line, buffer);
	}

	virtual ~LogSink() {
		Disable();
	}
};

//--------------------------------------

struct LogNoFilteringPolicy {
	bool filter(const LogLine *line) const { return true; }
};

struct LogNoDebugFilteringPolicy {
	bool filter(const LogLine *line) const { 
		switch (line->m_Mode) {
		case LogLineType::Debug:
		case LogLineType::DebugWarn:
			return false;
		default:
			return true;
		}
	}
};

struct LogFileOutputPolicy {
	~LogFileOutputPolicy() {
		m_file << "\n";
		m_file.close();
	}
	void open(const char*file, bool append = true) {
		m_file.open(file, std::ios::out | (append ? std::ios::app : 0));
	}
	void write(const LogLine *line, const char *c) {
		m_file << c << std::flush;
	}
protected:
	std::ofstream m_file;
};

struct LogStandardBannerWritter : public LogFileOutputPolicy {
	void open(const char*file, bool append = true);
};

struct LogStandardFormatter {
	LogStandardFormatter();
	void format(const LogLine *line, char* buffer, size_t buffer_size);
protected:
#ifdef _FEATURE_LOG_COUNTERS_
	unsigned NextLine() { return ++m_Line; }
	unsigned NextType(LogLineType t) { return ++m_Type[(unsigned)t]; }
	unsigned m_Line;
	unsigned m_Type[(unsigned)LogLineType::MaxValue];
#endif
};

using StdFileLoggerSink = LogSink <LogStandardBannerWritter, LogStandardFormatter, LogNoFilteringPolicy > ;
using StdNoDebugFileLoggerSink = LogSink <LogStandardBannerWritter, LogStandardFormatter, LogNoDebugFilteringPolicy > ;

//--------------------------------------

class LogEngine {
public:
	LogEngine();
	static void DeleteInstance();
	static LogEngine* Instance() { return _Instance; }

	bool IsRunning();

	static void Line(LogLineType level, unsigned line, const char *file, const char *fnc, const char* fmt, ...);
	static void Line(LogLineType level, unsigned line, const char *file, const char *fnc, std::ostringstream &ss);

#ifndef _DISABLE_LOG_SYSTEM_
	//static LogLineProxy BeginLine(LogLineType mode = LogLineType::None, unsigned line = 0, const char *file = 0, const char *fnc = 0) {
		//return LogLineProxy(new LogLine(mode, line, file, fnc));
	//}
#else
	static std::ostringstream BeginLine(unsigned line = 0, const char *file = 0, const char *fnc = 0, LogLineType mode = LogLineType::None) {
		return std::ostringstream();
	}
#endif
private:
	~LogEngine();
	static LogEngine* _Instance;

	struct LogEngineImpl;
	std::unique_ptr<LogEngineImpl> m_Impl;
};

#if defined(DEBUG) && defined(_BUILDING_ENGINE_)

struct StaticLogCatcher {
public:
	static void DispatchLog();
	enum class StaticLogType {
		Bug, FixMe, TODO, 
	};
protected:
	virtual void DispatchLine() const = 0;
	virtual StaticLogType GetType() const = 0;

	static void PushLog(StaticLogCatcher* log) {
		if (!_list)
			_list = new std::list < StaticLogCatcher* >() ;
		_list->push_back(log);
	}
private:
	static std::list<StaticLogCatcher*> *_list;
};

template<class info>
struct StaticLogLine : StaticLogCatcher {
	StaticLogLine() {
		PushLog(this);
	}
	~StaticLogLine() {
		_instance = nullptr;
	}
	void DispatchLine() const override {
		info i;
		i.ConstructMessage(
			::Log::LogEngine::BeginLine(::Log::LogLineType::Static, i.GetLine(), i.GetFile(), i.GetFunction()) 
			<< i.GetActionString() << ": " );			
	}
	virtual StaticLogType GetType() const { return info::GetAction(); }
	static StaticLogLine<info>* Get() {
		return _instance;
	}
private: 
	static StaticLogLine<info> *_instance;
};

template<class info>
StaticLogLine<info>* StaticLogLine<info>::_instance = new StaticLogLine<info>();

#define __Log_todo_action(T, ...) do { }while(false)
#define __Log_todo_action_disabled(T, ...)\
	{\
		static const char *__function = __FUNCTION__;\
		struct StaticLogLine_info {\
			using StaticLogType = ::Log::StaticLogCatcher::StaticLogType;\
			unsigned GetLine() { return m_line ; }\
			const char *GetFile() { return __FILE__ ; }\
			const char *GetFunction() { return __function; }\
			static StaticLogType GetAction() { return StaticLogType::T; }\
			const char *GetActionString() { return #T; }\
			void ConstructMessage(::Log::LogLineProxy &out) { out << __VA_ARGS__; }\
			StaticLogLine_info(unsigned l = __LINE__): m_line(l) { }\
		private:\
			unsigned m_line;\
		};\
		static auto __todo_item = ::Log::StaticLogLine<StaticLogLine_info>::Get(); \
	}

#else

#define __Log_todo_action(T, ...)					__disabled_log_action
	
#endif

} //namespace Log

#define __disabled_log_action						do { /* NOP */ } while(false)

#ifdef _DISABLE_LOG_SYSTEM_

#define __CreateLog(A)								__disabled_log_action
#define __BeginLog(TYPE, A)							__disabled_log_action
#define __BeginLogf(TYPE, FMT, ...)					__disabled_log_action
#define AddLogOnce(...)								__disabled_log_action
#define AddLogOncef(...)							__disabled_log_action

#else

#define __CreateLog(A, ...)							::Log::LogEngine::Line(::Log::LogLineType::A, __LINE__, __FILE__, __FUNCTION__, __VA_ARGS__)
#define __BeginLog(TYPE, A)							do { std::ostringstream __ss; __ss << A; __CreateLog(TYPE, __ss); } while(false)
#define __BeginLogf(TYPE, ...)						do { __CreateLog(TYPE, __VA_ARGS__); } while (false)
#define AddLogOnce(TYPE, A)							do { static bool ___Executed = false; if(!___Executed){ AddLog(TYPE, A); ___Executed = true; }} while (false)
#define AddLogOncef(TYPE, ...)						do { static bool ___Executed = false; if(!___Executed){ AddLogf(TYPE, __VA_ARGS__); ___Executed = true; }} while (false)

#endif

#define AddLog(T, A)								__LOG_ACTION_##T(T, A)
#define AddLogf(T, ...)								__LOG_ACTION_F_##T(T, __VA_ARGS__)

#define __LOG_ACTION_Normal(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Normal(T, ...)				__BeginLogf(T, __VA_ARGS__)
#define __LOG_ACTION_Error(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Error(T, ...)				__BeginLogf(T, __VA_ARGS__)
#define __LOG_ACTION_Warning(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Warning(T, ...)				__BeginLogf(T, __VA_ARGS__)
#define __LOG_ACTION_Hint(T, A)						__BeginLog(T, A)
#define __LOG_ACTION_F_Hint(T, ...)					__BeginLogf(T, __VA_ARGS__)

#define __LOG_ACTION_SysInfo(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_SysInfo(T, ...)				__BeginLogf(T, __VA_ARGS__)

#define __LOG_ACTION_Console(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Console(T, ...)				__BeginLogf(T, __VA_ARGS__)

#define __LOG_ACTION_InvalidEnum(T, V)				AddLogf(Error, "Invalid enum value (enum:'%s' value:%d)", typeid(V).name(), V)

#define __LOG_ACTION_TODO(T, ...)					__Log_todo_action(T, __VA_ARGS__)
#define __LOG_ACTION_Bug(T, ...)					__Log_todo_action(T, __VA_ARGS__)
#define __LOG_ACTION_FixMe(T, ...)					__Log_todo_action(T, __VA_ARGS__)

#ifdef DEBUG_LOG
#define __LOG_ACTION_Debug(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Debug(T, ...)				__BeginLogf(T, __VA_ARGS__)

#define __LOG_ACTION_DebugWarn(T, A)				__BeginLog(T, A)
#define __LOG_ACTION_F_DebugWarn(T, ...)			__BeginLogf(T, __VA_ARGS__)
#define __LOG_ACTION_Thread(T, NAME)				do { std::stringstream __ss; __ss << "Thread Info: ID:" << std::hex << std::this_thread::get_id() << " Name:" << NAME; __BeginLog(Hint, __ss.str()); } while(0)
#else
#define __LOG_ACTION_Debug(T, A)					__disabled_log_action
#define __LOG_ACTION_F_Debug(T, ...) 				__disabled_log_action
#define __LOG_ACTION_F_DebugCounter(T, ...)			__disabled_log_action
#define __LOG_ACTION_DebugWarn(T, A)				__disabled_log_action
#define __LOG_ACTION_F_DebugWarn(T, ...)			__disabled_log_action
#define __LOG_ACTION_Thread(T, A)					__disabled_log_action
#endif

#define CriticalCheck(COND, MSG)					do { if(!(COND)) { AddLogf(Error, "Critical check failed!!! condition '%s' returned false. Error message: '%s'", #COND, (MSG?MSG:"No error message")); throw MSG; } } while(0)

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_

#define __LOG_ACTION_F_Performance(T, ...)			__BeginLogf(T, __VA_ARGS__)
#define __LOG_ACTION_Performance(T, A)				__BeginLog(T, A)

namespace PerformanceCounters {
	inline void PrintPerfCounter(unsigned __int64 Value, void *OwnerPtr, const char *OwnerName, const char *Name) {
		AddLogf(Performance, "Destroying counter %s. Owner %s (%x). Current value: %llu", Name, OwnerName, OwnerPtr, static_cast<unsigned long long>(Value));
	}

	template<class OWNER, class INFO>
	struct Counter {
		OWNER *Owner = nullptr;
		unsigned __int64 Value = 0;
		void increment(unsigned __int64 val = 1) { Value += val; }
		void decrement(unsigned __int64 val = 1) { Value -= val; }
		void SetOwner(OWNER *ptr) {
			Owner = ptr;
		}
		~Counter() {
			const char *OwnerName;
			if (Owner)
				OwnerName = Owner->GetDynamicTypeInfo()->GetName();
			else 
				OwnerName = OWNER::GetStaticTypeInfo()->GetName();
			PrintPerfCounter(Value, Owner, OwnerName, INFO::Name());
		}
	};
}

#define DeclarePerformanceCounter(NAME)			\
private:										\
	struct __PerfCounterInfo_##NAME	{			\
		static const char* Name() { return #NAME; }\
	};											\
	PerformanceCounters::Counter<ThisClass, __PerfCounterInfo_##NAME> m_PerfCounter##NAME

#define IncrementPerformanceCounter(NAME)		do { m_PerfCounter##NAME.increment(); } while(0)
#define DecrementPerformanceCounter(NAME)		do { m_PerfCounter##NAME.decrement(); } while(0)
#define PerformanceCounter_inc(NAME, VALUE)		do { m_PerfCounter##NAME.increment(VALUE); } while(0)
#define PerformanceCounter_dec(NAME, VALUE)		do { m_PerfCounter##NAME.decrement(VALUE); } while(0)
#define SetPerformanceCounterOwner(NAME)		do { m_PerfCounter##NAME.SetOwner(this); } while(0)
#else
#define DeclarePerformanceCounter(NAME)	
#define IncrementPerformanceCounter(NAME)		do { /* nothing there */ } while(0)
#define DecrementPerformanceCounter(NAME)		do { /* nothing there */ } while(0)
#define SetPerformanceCounterOwner(NAME)		do { /* nothing there */ } while(0)
#define __LOG_ACTION_F_Performance(T, ...)		__disabled_log_action
#define __LOG_ACTION_Performance(T, A)			__disabled_log_action
#endif

#endif // CLOG_H_ 

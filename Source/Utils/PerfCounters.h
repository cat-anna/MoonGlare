#pragma once

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_

#define __LOG_ACTION_F_Performance(T, ...)			ORBITLOGGER_BeginLogf(Hint, 0, __VA_ARGS__)
#define __LOG_ACTION_Performance(T, A)				ORBITLOGGER_BeginLog(Hint, 0, A)

namespace PerformanceCounters {
inline void PrintPerfCounter(unsigned __int64 Value, void *OwnerPtr, const char *OwnerName, const char *Name) {
	//AddLogf(Performance, "Destroying counter %s. Owner %s (%x). Current value: %llu", Name, OwnerName, OwnerPtr, static_cast<unsigned long long>(Value));
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
#define __LOG_ACTION_F_Performance(T, ...)		ORBITLOGGER_DISASBLED_ACTION()
#define __LOG_ACTION_Performance(T, A)			ORBITLOGGER_DISASBLED_ACTION()
#endif


#define DISABLE_COPY() public: ThisClass(const ThisClass&) = delete; ThisClass& operator=(const ThisClass&) = delete

#define AS_STRING(X) #X

#ifdef DEBUG
#define ERROR_STR				"{badstr in " __FUNCTION__ " at " AS_STRING(__LINE__) "}"
#else
#define ERROR_STR				"{?}"
#endif


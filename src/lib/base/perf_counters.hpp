#pragma once

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_

#define __LOG_ACTION_F_Performance(T, ...)			ORBITLOGGER_BeginLogf(Hint, 0, __VA_ARGS__)
#define __LOG_ACTION_Performance(T, A)				ORBITLOGGER_BeginLog(Hint, 0, A)

namespace PerformanceCounters {
inline void PrintPerfCounter(unsigned __int64 Value, void *OwnerPtr, const char *OwnerName, const char *Name) {
	if (OwnerPtr)
		AddLogf(Performance, "Destroying counter %s. Owner %s (%p). Value: %llu", Name, OwnerName, OwnerPtr, static_cast<uint64_t>(Value));
	else
		AddLogf(Performance, "Destroying counter %s. Owner %s. Value: %llu", Name, OwnerName, static_cast<uint64_t>(Value));
}
template<typename Mode, typename OWNER>
struct GetCouterOwnerName;

template<class OWNER, class INFO>
struct Counter {
	std::atomic<uint64_t> Value = 0;
	OWNER *Owner = nullptr;
	std::string OwnerName = typeid(OWNER).name();
	void increment(uint64_t val = 1) { Value += val; }
	void decrement(uint64_t val = 1) { Value -= val; }

	void SetOwner(OWNER *ptr) {
		Owner = ptr;
		OwnerName = typeid(*ptr).name();
	}
	~Counter() {
		PrintPerfCounter(Value, Owner, OwnerName.c_str(), INFO::Name());
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
#endif

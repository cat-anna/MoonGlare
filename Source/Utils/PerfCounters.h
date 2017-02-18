#pragma once

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_

#define __LOG_ACTION_F_Performance(T, ...)			ORBITLOGGER_BeginLogf(Hint, 0, __VA_ARGS__)
#define __LOG_ACTION_Performance(T, A)				ORBITLOGGER_BeginLog(Hint, 0, A)

namespace PerformanceCounters {
inline void PrintPerfCounter(unsigned __int64 Value, void *OwnerPtr, const char *OwnerName, const char *Name) {
	if (OwnerPtr)
		AddLogf(Performance, "Destroying counter %s. Owner %s (%p). Current value: %llu", Name, OwnerName, OwnerPtr, static_cast<unsigned long long>(Value));
	else
		AddLogf(Performance, "Destroying counter %s. Owner %s. Current value: %llu", Name, OwnerName, static_cast<unsigned long long>(Value));
}
template<typename Mode, typename OWNER>
struct GetCouterOwnerName;

template<class OWNER, class INFO>
struct Counter {
	std::atomic<uint64_t> Value = 0;
	OWNER *Owner = nullptr;
	std::string OwnerName = GetCouterOwnerName<std::is_base_of<cRootClass, OWNER>::type, OWNER>::Get(nullptr);
	void increment(uint64_t val = 1) { Value += val; }
	void decrement(uint64_t val = 1) { Value -= val; }

	void SetOwner(OWNER *ptr) {
		Owner = ptr;
		OwnerName = GetCouterOwnerName<std::is_base_of<cRootClass, OWNER>::type, OWNER>::Get(ptr);
	}
	~Counter() {
		PrintPerfCounter(Value, Owner, OwnerName.c_str(), INFO::Name());
	}
};

template<typename OWNER>
struct GetCouterOwnerName<std::true_type, OWNER> {
	static std::string Get(OWNER *Owner) {
		if (Owner)
			return Owner->GetDynamicTypeInfo()->GetName();
		else
			return OWNER::GetStaticTypeInfo()->GetName();
	}
};

template<typename OWNER>
struct GetCouterOwnerName<std::false_type, OWNER> {
	static std::string Get(OWNER *Owner) {
		return typeid(OWNER).name();
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


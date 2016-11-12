#pragma once
#ifndef cTimeEventsH
#define cTimeEventsH

namespace MoonGlare {
namespace Core {

#if 0
class TimeEvents : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(TimeEvents, cRootClass)
public:
	TimeEvents();
	~TimeEvents();

	void Clear();
	void CheckEvents(const MoveConfig &conf);

	int SetTimer(int EventID, float ElapseTime, bool Cyclic, EventProxyPtr Owner);
	int KillTimer(int EventID, EventProxyPtr Owner);
	void KillTimersForObject(EventProxyPtr Owner);
protected:
	struct TimeEventInfo {
		float EndTime = 0;
		float ElapseTime = 0;
		int EventID = 0;
		EventProxyPtr Owner;
		bool Cyclic = false;

		TimeEventInfo(float end, float elapse, int id, EventProxyPtr &owner, bool cyclic) :
			EndTime(end), ElapseTime(elapse), EventID(id), Owner(owner), Cyclic(cyclic) { }

		void ResetTime(float CurrentTime) { EndTime = CurrentTime + ElapseTime; }
		void ResetCycle() { EndTime += ElapseTime; }
		bool operator <(const TimeEventInfo &Other) const { return EndTime < Other.EndTime; }
	};

	typedef std::list<TimeEventInfo> TimeEventsQueue;

	float m_CurrentTime = 0;
	TimeEventsQueue m_Queue;
};
#endif // 0


} //namespace Core
} //namespace MoonGlare 

#endif

#include <pch.h>
#include <MoonGlare.h>

#include <algorithm>

namespace MoonGlare {
namespace Core {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(TimeEvents)

TimeEvents::TimeEvents(){
}

TimeEvents::~TimeEvents(){
}

void TimeEvents::Clear(){
	m_Queue.clear();
	m_CurrentTime = 0;
}

void TimeEvents::KillTimersForObject(EventProxyPtr Owner){
	auto req = Owner.lock();
	for (auto it = m_Queue.begin(), jt = m_Queue.end(); it != jt; ++it) {
		if (it->Owner.expired() || it->Owner.lock() == req) {
			auto del = it++;
			m_Queue.erase(del);
			if (it == jt)break;
		}
	}
}

void TimeEvents::CheckEvents(const MoveConfig &conf){
	m_CurrentTime += conf.TimeDelta;
	while(!m_Queue.empty()){
		auto &item = m_Queue.front();
		if(m_CurrentTime < item.EndTime) return;

		if (item.Owner.expired()) {
			m_Queue.pop_front();
			continue;
		}

		TimeEventInfo info = item;
		m_Queue.pop_front();
		if (info.Cyclic) {
			info.ResetCycle();
			m_Queue.insert(std::lower_bound(m_Queue.begin(), m_Queue.end(), info), info);
		} 
		info.Owner.lock()->TimerEvent(info.EventID);
	}
}

int TimeEvents::SetTimer(int EventID, float ElapseTime, bool Cyclic, EventProxyPtr Owner){
	if (ElapseTime <= 0) {
		AddLog(Warning, "Attempt to set timer with negative elapse time!");
		return 0;
	}
	TimeEventInfo info{ m_CurrentTime + ElapseTime, ElapseTime, EventID, Owner, Cyclic };
	m_Queue.insert(std::lower_bound(m_Queue.begin(), m_Queue.end(), info), info);
	return EventID;
}

int TimeEvents::KillTimer(int Event, EventProxyPtr Owner){
	auto req = Owner.lock();
	for (auto it = m_Queue.begin(), jt = m_Queue.end(); it != jt; ++it) {
		if (it->Owner.expired() || it->EventID != Event) continue;
		auto ptr = it->Owner.lock();
		if (ptr != req) continue;
		m_Queue.erase(it);
		return 0;
	}
	AddLogf(Warning, "Timer with id %d does not exists!", Event);
	return 0;
}

} //namespace Core
} //namespace MoonGlare 

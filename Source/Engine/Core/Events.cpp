s/*
	Generated by cppsrc.sh
	On 2015-01-20  9:04:58,62
	by Paweu
*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "Events.h"

namespace MoonGlare {
namespace Core {
namespace Events {

SPACERTTI_IMPLEMENT_STATIC_CLASS(EventBaseProxy)

EventBaseProxy::EventBaseProxy() {}
EventBaseProxy::~EventBaseProxy() {}

int EventBaseProxy::TimerEvent(int TimerID) { return 0; };
int EventBaseProxy::InternalEvent(InternalEvents event, int Param) { return 0; };

//---------------------------------------------------------------------------------------

} //namespace Events
} //namespace Core 
} //namespace MoonGlare 

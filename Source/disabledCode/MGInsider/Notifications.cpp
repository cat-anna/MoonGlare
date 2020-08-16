#include PCH_HEADER
#include "Notifications.h"

Notifications::Notifications()
{
}

Notifications::~Notifications()
{
}

//-----------------------------------------

struct NotificationsImpl : public Notifications {
};

Notifications* Notifications::Get() {
	static NotificationsImpl impl;
	return &impl;
}

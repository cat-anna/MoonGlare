#include PCH_HEADER
#include "Notifications.h"

namespace MoonGlare {
namespace Editor {

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

} //namespace Editor
} //namespace MoonGlare

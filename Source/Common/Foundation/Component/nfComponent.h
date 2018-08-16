#pragma once

#include <pugixml.hpp>

#include "../InterfaceMap.h"
#include "Entity.h"

namespace MoonGlare::Component {

struct SubsystemUpdateData;

class iSubsystemManager;
class iSubsystem;
using UniqueSubsystem = std::unique_ptr<iSubsystem>;

class EventDispatcher;

}

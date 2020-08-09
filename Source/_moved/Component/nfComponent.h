#pragma once

#include <pugixml.hpp>

#include "../InterfaceMap.h"
#include "Entity.h"

namespace MoonGlare::Component {

class EntityManager;
class EventDispatcher;

struct SubsystemUpdateData;

class iSubsystemManager;
class iSubsystem;
using UniqueSubsystem = std::unique_ptr<iSubsystem>;


}

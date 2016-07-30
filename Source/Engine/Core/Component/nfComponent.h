#pragma once
#ifndef nfComponent_H
#define nfComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

using ComponentID = uint32_t;

//inline ComponentID make_componentid(uint32_t group, uint32_t id) { return group << 16 | (id & 0xFFFF); }
#define MAKE_COMPONENTID(group, id) ( ((group) & 0xFFFF) << 16 | ((id) & 0xFFFF) )

class AbstractComponent;
using UniqueAbstractComponent = std::unique_ptr<AbstractComponent>;

class ComponentManager;

class ScriptComponent;
class TransformComponent;

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

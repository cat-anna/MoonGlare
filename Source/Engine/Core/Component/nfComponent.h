#pragma once
#ifndef nfComponent_H
#define nfComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

using ComponentID = uint32_t;

class AbstractComponent;
using UniqueAbstractComponent = std::unique_ptr<AbstractComponent>;

class ComponentManager;

class ScriptComponent;
class TransformComponent;

enum class ComponentIDs : ComponentID {
	Invalid = 0,

	Script = 11,
	Transform = 12,

	Mesh	= 21,
};

//template<ComponentIDs ID, HandleType HT>
//struct ComponentIDWrap {
//	constexpr static ComponentID GetComponentID() { return static_cast<ComponentID>(ID); };
//	constexpr static HandleType GetHandleType() { return static_cast<HandleType>(HT); };
//};

template<ComponentIDs VALUE>
struct ComponentIDWrap {
	constexpr static ComponentID GetComponentID() { return static_cast<ComponentID>(VALUE); };
	constexpr static HandleType GetHandleType() { return static_cast<HandleType>(VALUE); };
};

} //namespace Component 

using Component::ComponentID;

} //namespace Core 
} //namespace MoonGlare 

#endif

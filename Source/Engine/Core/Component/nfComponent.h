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
class CameraComponent;

enum class ComponentIDs : ComponentID {
	Invalid = 0,

//Core - 1X
	Script = 11,
	Transform = 12,

//Renderer - 2X
	Mesh = 21,
	Light = 22,
	Camera = 23,

//Renderer - 5X
	Body = 50,
	BodyShape = 51,
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

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

//Core - 0x1X
	Script = 0x11,
	Transform = 0x12,

//Renderer - 0x2X
	Mesh = 0x21,
	Light = 0x22,
	Camera = 0x23,

//Physics - 0x5X
	Body = 0x50,
	BodyShape = 0x51,

//2d - 0x6x
	RectTransform = 0x60,
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

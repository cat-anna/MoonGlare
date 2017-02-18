#pragma once
#ifndef nfComponent_H
#define nfComponent_H

#define DEFINE_COMPONENT_PROPERTY(NAME)						\
decltype(m_##NAME) Get ## NAME() const { return m_##NAME; }	\
void Set ## NAME(const decltype(m_##NAME) &v) { m_##NAME = v; SetDirty(); }	

#define DEFINE_COMPONENT_PROPERTY_CAST(NAME, TYPE)					\
TYPE Get ## NAME() const { return static_cast<TYPE>(m_##NAME); }	\
void Set ## NAME(const TYPE &v) { m_##NAME = static_cast<decltype(m_##NAME)>(v); SetDirty(); }	

#define DEFINE_COMPONENT_FLAG(NAME, LOCATION)				\
bool Get ## NAME() const { return LOCATION; }				\
void Set ## NAME(bool v) {LOCATION = v; SetDirty(); }	

namespace MoonGlare {
namespace Core {
namespace Component {

class AbstractComponent;
using UniqueAbstractComponent = mem::aligned_ptr<AbstractComponent>;

class ComponentManager;

class TransformComponent;

enum class ComponentID : uint8_t {
	Invalid = 0,

//Core - 0x1X
	Script = 0x11,
	Transform = 0x12,

//Renderer - 0x2X
	Mesh = 0x21,
	Light = 0x22,
	Camera = 0x23,

	DirectAnimation = 0x2F,

//Physics - 0x5X
	Body = 0x50,
	BodyShape = 0x51,

//GUI				  0x6x
	RectTransform	= 0x60,
	Image			= 0x61,
	Panel			= 0x62,
	Text			= 0x63,
};

//template<ComponentID ID, HandleType HT>
//struct ComponentIDWrap {
//	constexpr static ComponentID GetComponentID() { return static_cast<ComponentID>(ID); };
//	constexpr static HandleType GetHandleType() { return static_cast<HandleType>(HT); };
//};

template<ComponentID VALUE>
struct ComponentIDWrap {
	constexpr static ComponentID GetComponentID() { return VALUE; };
	constexpr static ComponentID GetHandleType() { return VALUE; };
};

} //namespace Component 

using Component::ComponentID;

} //namespace Core 
} //namespace MoonGlare 

#endif

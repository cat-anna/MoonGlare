#pragma once
#ifndef ComponentIDsH
#define ComponentIDsH

namespace MoonGlare {
namespace Core {
namespace Component {

enum class ComponentIDs : ComponentID {
	Invalid = 0,

	Script = 1,
	Transform = 2,
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
} //namespace Core 
} //namespace MoonGlare 

#endif

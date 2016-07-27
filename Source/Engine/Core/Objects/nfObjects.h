#pragma once
#ifndef NFOBJECTS_H
#define NFOBJECTS_H

namespace MoonGlare {
namespace Core {
namespace Objects {
	class Object;
	class ObjectList;
	class ObjectRegister;

	using UniqueObjectRegister = std::unique_ptr<ObjectRegister>;

	class iLightSource;
	using iLightSourcePtr = std::unique_ptr < iLightSource > ;

	using LightSoureClassRegister = Space::DynamicClassRegister < iLightSource, Object* > ;
}
} //namespace Core
} //namespace MoonGlare 

using MoonGlare::Core::Objects::Object;
using MoonGlare::Core::Objects::iLightSource; 

#endif

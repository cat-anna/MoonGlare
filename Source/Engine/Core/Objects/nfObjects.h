#pragma once
#ifndef NFOBJECTS_H
#define NFOBJECTS_H

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

using ::Core::Objects::Object;
using ::Core::Objects::iLightSource; 

#endif

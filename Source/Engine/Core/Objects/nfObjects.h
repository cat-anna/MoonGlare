#pragma once
#ifndef NFOBJECTS_H
#define NFOBJECTS_H

namespace Core {
	namespace Objects {
		class Object;
		class ObjectList;
		class ObjectRegister;

		class StaticObject;
		class DynamicObject;

		class Player;

		class iLightSource;
		using iLightSourcePtr = std::unique_ptr < iLightSource > ;

		using LightSoureClassRegister = GabiLib::DynamicClassRegister < iLightSource, Object* > ;
		using ObjectClassRegister = GabiLib::DynamicClassRegister < Object, GameScene* > ;
	}
} //namespace Core

using ::Core::Objects::Object;
using ::Core::Objects::iLightSource; 

#endif

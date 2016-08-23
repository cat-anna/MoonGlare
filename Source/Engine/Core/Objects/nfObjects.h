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
}
} //namespace Core
} //namespace MoonGlare 

using MoonGlare::Core::Objects::Object;

#endif

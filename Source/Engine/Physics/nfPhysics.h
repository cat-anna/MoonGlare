#pragma once
#ifndef NFPHYSICSH
#define NFPHYSICSH

namespace MoonGlare {
namespace Physics {

} //namespace Physics
} //namespace MoonGlare

namespace Physics {

	using namespace ::MoonGlare::Physics;

	class BulletDebugDrawer;

	class PhysicEngine;
	using PhysicEnginePtr = std::unique_ptr < PhysicEngine > ;

	class Material;

	class Body;
	using BodyPtr = std::unique_ptr < Body > ;

	class ShapeSubBase;
	using Shape = ShapeSubBase;
	using SharedShape = std::shared_ptr < Shape > ;

	class ShapeConstructor;
	using ShapeConstructorPtr = std::unique_ptr < ShapeConstructor > ;

	typedef btMotionState MotionState;

	class ShapeSubBase;
	class BoxShape;

	using vec3 = btVector3;
	using Quaternion = btQuaternion;
	using Transform = btTransform;

	struct PhysicalProperties;

	using BodyShapeClassRegister = Space::DynamicClassRegister < ShapeConstructor > ;

} //namespace Physics

namespace MoonGlare {
namespace Physics {
namespace Component {

class BodyComponent;
class BodyShapeComponent;

} //namespace Component

using namespace ::Physics;

} //namespace Physics
} //namespace MoonGlare

using ::Physics::vec3;

#endif
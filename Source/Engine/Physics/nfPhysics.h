#pragma once
#ifndef NFPHYSICSH
#define NFPHYSICSH

namespace Physics {
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

	using BodyShapeClassRegister = GabiLib::DynamicClassRegister < ShapeConstructor > ;

} //namespace Physics

using Physics::vec3;

#endif
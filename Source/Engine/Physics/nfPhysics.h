#pragma once

namespace MoonGlare {
namespace Physics {

} //namespace Physics
} //namespace MoonGlare

namespace Physics {
	using namespace ::MoonGlare::Physics;

	class BulletDebugDrawer;

	typedef btMotionState MotionState;

	using vec3 = btVector3;
	using Quaternion = btQuaternion;
	using Transform = btTransform;
} //namespace Physics

namespace MoonGlare {
namespace Physics {
namespace Component {

class BodyComponent;
class BodyShapeComponent;

struct OnCollisionEnterEvent;
struct OnCollisionLeaveEvent;

} //namespace Component

using namespace ::Physics;

} //namespace Physics
} //namespace MoonGlare

using ::Physics::vec3;

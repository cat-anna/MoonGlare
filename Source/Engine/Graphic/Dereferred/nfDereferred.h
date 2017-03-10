#ifndef NFDEREFERRED_H
#define NFDEREFERRED_H

namespace Graphic {
namespace Dereferred {

class DereferredFrameBuffer;
class DereferredPipeline;

struct SamplerIndex {
	enum {
		Empty = 0,
		Position,
		Diffuse,
		Normal,
		PlaneShadow,

	};
};

} // namespace Dereferred
} // namespace Graphic

#endif // GRAPHICMATERIAL_H

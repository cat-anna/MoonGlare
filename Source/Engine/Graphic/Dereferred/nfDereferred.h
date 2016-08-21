#ifndef NFDEREFERRED_H
#define NFDEREFERRED_H

namespace Graphic {
namespace Dereferred {

class SpotLightShader;
class PointLightShader;
class DirectionalLightShader;
class LightingPassShader;
class GeometryPassShader;

struct SamplerIndex {
	enum {
		Empty = 0,
		Position,
		Diffuse,
		Normal,

		PlaneShadow = 5,
	};
};


} // namespace Dereferred
} // namespace Graphic

#endif // GRAPHICMATERIAL_H

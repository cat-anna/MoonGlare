#pragma once

namespace MoonGlare::Renderer::Deferred {

class DeferredFrameBuffer;
class DeferredPipeline;

struct SamplerIndex {
	enum {
		Empty = 0,
		Position,
		Diffuse,
		Normal,
        Shadow,
        //CubeShadow,
	};
};

struct DeferredSink;

} 
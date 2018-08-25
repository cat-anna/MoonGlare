#pragma once

namespace MoonGlare::Renderer::Deferred {

struct StencilLightShaderDescriptor {
	enum class InLayout {
		Position,
		MaxValue,
	};
	enum class OutLayout {
		MaxValue,
	};
	enum class Uniform {
		CameraMatrix,
		ModelMatrix,
		MaxValue,
	};
	enum class Sampler {
		MaxValue,
	};

	constexpr static const char* GetName(Uniform u) {
		switch (u) {
		case Uniform::CameraMatrix: return "CameraMatrix";
		case Uniform::ModelMatrix: return "ModelMatrix";
		default: return nullptr;
		}
	}
	constexpr static const char* GetSamplerName(Sampler s) {
		//switch (s) {
		//default: 
			return nullptr;
		//}
	}
};

} 
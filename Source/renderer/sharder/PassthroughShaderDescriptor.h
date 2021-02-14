#pragma once

namespace MoonGlare::Renderer {

struct PassthroughShaderDescriptor {
	enum class InLayout {
		Position,
		TextureUV,
		Normal,
	};
	enum class OutLayout {
		FragColor,
	};
	enum class Uniform {
		CameraMatrix,
		ModelMatrix,
		BackColor,

		MaxValue,
	};
	enum class Sampler {
		DiffuseMap,
		MaxValue,
	};

	constexpr static const char* GetName(Uniform u) {
		switch (u) {
		case Uniform::CameraMatrix: return "gCameraMatrix";
		case Uniform::ModelMatrix: return "gModelMatrix";
		case Uniform::BackColor: return "gBackColor";
		default: return nullptr;
		}
	}
	constexpr static const char* GetSamplerName(Sampler s) {
		switch (s) {
		case Sampler::DiffuseMap: return "Texture0";
		default: return nullptr;
		}
	}
};

}

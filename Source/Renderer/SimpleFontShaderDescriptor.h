#pragma once

namespace MoonGlare::Renderer{

struct SimpleFontShaderDescriptor {
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

	constexpr static const char* GetName(Uniform u) {
		switch (u) {
		case Uniform::CameraMatrix: return "CameraMatrix";
		case Uniform::ModelMatrix: return "ModelMatrix";
		case Uniform::BackColor: return "Material.BackColor";
		default: return nullptr;
		}
	}
};

}

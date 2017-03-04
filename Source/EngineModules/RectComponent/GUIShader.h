/*
  * Generated by cppsrc.sh
  * On 2016-09-25 17:31:20,49
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef GUIShader_H
#define GUIShader_H

namespace MoonGlare {
namespace GUI {

struct GUIShaderDescriptor {
	enum class InLayout {
		Position,
	};
	enum class OutLayout {
		FragColor,
	};
	enum class Uniform {
		CameraMatrix,
		ModelMatrix,
		BaseColor,
		TileMode,
		Border,
		PanelSize,
		PanelAspect,
		MaxValue,
	};
	enum class Sampler {
		DiffuseMap,
		MaxValue,
	};

	constexpr static const char* GetName(Uniform u) {
		switch (u) {
		case Uniform::CameraMatrix: return "CameraMatrix";
		case Uniform::ModelMatrix: return "ModelMatrix";
		case Uniform::BaseColor: return "gBaseColor";
		case Uniform::TileMode: return "gTileMode";
		case Uniform::Border: return "gPanelBorder";
		case Uniform::PanelSize: return "gPanelSize";
		case Uniform::PanelAspect: return "gPanelAspect";

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

} //namespace GUI 
} //namespace MoonGlare 

#endif

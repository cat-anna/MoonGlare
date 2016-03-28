#ifndef GRAPHICSETTINGS_H
#define GRAPHICSETTINGS_H

namespace Graphic {
namespace Settings {

enum class FinteringMode {
	Nearest, 
	Linear,
	Bilinear,
	Trilinear,

	Value_Max,
	Value_Min = Nearest,
};

enum class ShadowQuality {
	Disabled,
	Low,
	Medium,
	High,
	Ultra,

	Value_Max,
	Value_Min = Disabled,
};

inline unsigned GetShadowMapSize(ShadowQuality quality) {
	//dumb values, they are subject to tests
	switch (quality) {
	case ShadowQuality::Low: return 256;
	case ShadowQuality::Medium: return 512;
	case ShadowQuality::High: return 1024;
	case ShadowQuality::Ultra: return 2048;
	case ShadowQuality::Disabled:
	default:
		return 1;
	}
}

inline float GetLightFadeOffThreshold() { return 0.05f; }

} // namespace Settings
} // namespace Graphic
#endif // GRAPHICSETTINGS_H

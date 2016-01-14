#pragma once

namespace MoonGlare {
namespace GUI {

struct Margin {
	float Left = 0;
	float Right = 0;
	float Top = 0;
	float Bottom = 0;

	Margin() {}
	Margin(float l, float r, float t, float b): Left(l), Right(r), Top(t), Bottom(b) { }
	Margin(const math::vec4 &vec): Left(vec[0]), Right(vec[1]), Top(vec[2]), Bottom(vec[2]) { }
	Margin(float value) { Set(value); }

	float HorizontalMargin() const { return Left + Right; }
	float VerticalMargin() const { return Top + Bottom; }

	void Set(float val) { Left = Right = Top = Bottom = val; }
};

} //namespace GUI 
} //namespace MoonGlare 
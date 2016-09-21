#pragma once

namespace MoonGlare {
namespace GUI {

struct Margin {
	union {
		struct {
			float Left;
			float Right;
			float Top;
			float Bottom;
		};
		struct {
			float m_Left;
			float m_Right;
			float m_Top;
			float m_Bottom;
		};
	};

	Margin() {
		Left	= 0;
		Right	= 0;
		Top		= 0;
		Bottom  = 0;
	}
	Margin(float l, float r, float t, float b): Left(l), Right(r), Top(t), Bottom(b) { }
	Margin(const math::vec4 &vec): Left(vec[0]), Right(vec[1]), Top(vec[2]), Bottom(vec[2]) { }
	Margin(float value) { Set(value); }

	float HorizontalMargin() const { return Left + Right; }
	float VerticalMargin() const { return Top + Bottom; }

	Point LeftTopMargin() const { return Point(Left, Top); }
	Point RightBottomMargin() const { return Point(Right, Bottom); }
	Point TotalMargin() const { return Point(HorizontalMargin(), VerticalMargin()); }

	Margin operator / (const Point &div) const { return Margin(Left / div.x, Right / div.x, Top / div.y, Bottom / div.y); }
	const Margin& operator /= (const Point &div) { return Left /= div.x, Right /= div.x, Top /= div.y, Bottom /= div.y, *this; }
	Margin operator * (const Point &div) const { return Margin(Left * div.x, Right * div.x, Top * div.y, Bottom * div.y); }
	const Margin& operator *= (const Point &div) { return Left *= div.x, Right *= div.x, Top *= div.y, Bottom *= div.y, *this; }

	void Set(float val) { Left = Right = Top = Bottom = val; }
};

} //namespace GUI 
} //namespace MoonGlare 
#pragma once

namespace MoonGlare {
namespace GUI {

struct Rect {
	Point LeftTop;
	Point RightBottom;

	void Set(const Point& pos, const Point &size) {
		LeftTop = pos;
		RightBottom = pos + size;
	}

	Rect(): LeftTop(0), RightBottom(0) { }

	void SliceFromParent(const Rect& parent, const Point& pos, const Point &size) {
//		AddLog(FixMe, "Function implementation does not cover all cases");
		LeftTop = pos + parent.LeftTop;
		RightBottom = LeftTop + size;
	}

	bool IsPointInside(const Point& pos) const {
		if (pos.x < LeftTop.x) return false;
		if (pos.y < LeftTop.y) return false;
		if (pos.x > RightBottom.x) return false;
		if (pos.y > RightBottom.y) return false;
		return true;
	}
};

} //namespace GUI 
} //namespace MoonGlare 

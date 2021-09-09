#pragma once

#include "../component_common.hpp"
#include "math/vector.hpp"
#include "rect_enums.hpp"
#include <cstddef>

namespace MoonGlare::Component::Rect {

#if 0
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

	Margin operator / (const Point &div) const { return Margin(Left / div.x, Right / div.x, Top / div.y, Bottom / div.y); }
	const Margin& operator /= (const Point &div) { return Left /= div.x, Right /= div.x, Top /= div.y, Bottom /= div.y, *this; }
	Margin operator * (const Point &div) const { return Margin(Left * div.x, Right * div.x, Top * div.y, Bottom * div.y); }
	const Margin& operator *= (const Point &div) { return Left *= div.x, Right *= div.x, Top *= div.y, Bottom *= div.y, *this; }

	void Set(float val) { Left = Right = Top = Bottom = val; }
};

#pragma once

namespace MoonGlare {
namespace GUI {

using Point = ::glm::fvec2;

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

	Point GetSize() const { return RightBottom - LeftTop; }
	void SetSize(const Point &size) { RightBottom = LeftTop + size; }

	void SetPositionSize(const Point &pos, const Point &size) { LeftTop = pos; RightBottom = pos + size; }
};

} //namespace GUI
} //namespace MoonGlare

#endif

// Margin order: Left Right Top Bottom

using Margin = math::fvec4;
using Point = math::fvec2;
using Rect = math::fvec4;

static constexpr size_t kMarginIndexLeft = 0;
static constexpr size_t kMarginIndexRight = 1;
static constexpr size_t kMarginIndexTop = 2;
static constexpr size_t kMarginIndexBottom = 3;

inline float Horizontal(const Margin &margin) {
    return margin[kMarginIndexLeft] + margin[kMarginIndexTop];
}
inline float Vertical(const Margin &margin) {
    return margin[kMarginIndexTop] + margin[kMarginIndexBottom];
}
inline Point LeftTop(const Margin &margin) {
    return Point(margin[kMarginIndexLeft], margin[kMarginIndexTop]);
}
inline Point RightBottom(const Margin &margin) {
    return Point(margin[kMarginIndexRight], margin[kMarginIndexBottom]);
}
inline Point TotalMargin(const Margin &margin) {
    return Point(Horizontal(margin), Vertical(margin));
}

inline Point GetRectSize(const Rect &rect) {
    return LeftTop(rect) - RightBottom(rect);
}

struct alignas(16) RectTransform : public ComponentBase<RectTransform> {
    static constexpr ComponentId kComponentId = 16;
    static constexpr char kComponentName[] = "rect_transform";
    static constexpr bool kEditable = true;
    static constexpr bool kSerializable = true;

    // RectTransformComponentEntryFlagsMap m_Flags;

    ComponentRevision revision;

    AlignMode align_mode;

    math::fvec3 position;
    math::fvec3 size;
    Margin margin;

    // math::mat4 m_GlobalMatrix;
    // math::mat4 m_LocalMatrix;
    // Rect screen_rect;

    // MoonGlare::Configuration::RuntimeRevision m_Revision;

    // void Recalculate(RectTransformComponentEntry &Parent);

    void SetDirty() { revision = 0; }

    // void Reset() {
    //     m_Revision = 0;
    //     m_Flags.ClearAll();
    // }

    // std::string String() const { return fmt::format(""); }
};

static_assert((sizeof(RectTransform) % 16) == 0);
// static_assert((offsetof(RectTransform, scale) % 16) == 0);
// static_assert((offsetof(RectTransform, position) % 16) == 0);
// static_assert((offsetof(RectTransform, quaternion) % 16) == 0);

#ifdef WANTS_TYPE_INFO

auto GetTypeInfo(RectTransform *) {
    return AttributeMapBuilder<RectTransform>::Start(RectTransform::kComponentName)
        ->AddField("align_mode", &RectTransform::align_mode)
        ->AddField("position", &RectTransform::position)
        ->AddField("size", &RectTransform::size)
        ->AddField("margin", &RectTransform::margin)
        //
        ;
}

#endif

#ifdef WANTS_SERIALIZATION

void to_json(nlohmann::json &j, const RectTransform &p) {
    j = {
        {"align_mode", p.align_mode},
        {"position", p.position},
        {"size", p.size},
        {"margin", p.margin},
    };
}
void from_json(const nlohmann::json &j, RectTransform &p) {
    j.at("align_mode").get_to(p.align_mode);
    j.at("position").get_to(p.position);
    j.at("size").get_to(p.size);
    j.at("margin").get_to(p.margin);
    p.SetDirty();
}

#endif

} // namespace MoonGlare::Component::Rect

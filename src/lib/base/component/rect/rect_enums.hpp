#pragma once

#include "enum_converter.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>

namespace MoonGlare::Component::Rect {

enum class AlignMode : uint8_t {
    kNone,
    kTop,
    kBottom,
    kLeft,
    kRight,
    kLeftTop,
    kLeftBottom,
    kRightTop,
    kRightBottom,
    kLeftMiddle,
    kRightMiddle,
    kMiddleTop,
    kMiddleBottom,
    kFillParent,
    kCenter,
    kTable,
};

struct AlignModeEnumConverter : EnumConverter<AlignMode, AlignMode::kNone> {
    AlignModeEnumConverter() {
        Add("Default", Enum::kNone);
        Add("Parent", Enum::kFillParent);
        Add("FillParent", Enum::kFillParent);

        Add("Center", Enum::kCenter);
        Add("Top", Enum::kTop);
        Add("Bottom", Enum::kBottom);
        Add("Left", Enum::kLeft);
        Add("Right", Enum::kRight);
        Add("LeftMiddle", Enum::kLeftMiddle);
        Add("RightMiddle", Enum::kRightMiddle);
        Add("MiddleTop", Enum::kMiddleTop);
        Add("MiddleBottom", Enum::kMiddleBottom);

        Add("LeftTop", Enum::kLeftTop);
        Add("LeftBottom", Enum::kLeftBottom);
        Add("RightTop", Enum::kRightTop);
        Add("RightBottom", Enum::kRightBottom);

        Add("Table", Enum::kTable);
    }
};
using AlignModeEnum = EnumConverterHolder<AlignModeEnumConverter>;
void from_json(const nlohmann::json &j, AlignMode &mode);

//----------------------------------------------------------------------------------

enum class TextAlignMode : uint8_t {
    kLeftTop,
    kMiddleTop,
    kRightTop,
    kLeftMiddle,
    kMiddle,
    kRightMiddle,
    kLeftBottom,
    kMiddleBottom,
    kRightBottom,
    //kJustified, //:)
};
struct TextAlignModeEnumConverter : EnumConverter<TextAlignMode, TextAlignMode::kLeftTop> {
    TextAlignModeEnumConverter() {
        Add("Default", Enum::kLeftTop);
        //Add("Justified", Enum::kJustified);
        Add("LeftTop", Enum::kLeftTop);
        Add("MiddleTop", Enum::kMiddleTop);
        Add("RightTop", Enum::kRightTop);
        Add("LeftMiddle", Enum::kLeftMiddle);
        Add("Middle", Enum::kMiddle);
        Add("RightMiddle", Enum::kRightMiddle);
        Add("LeftBottom", Enum::kLeftBottom);
        Add("MiddleBottom", Enum::kMiddleBottom);
        Add("RightBottom", Enum::kRightBottom);
    }
};
using TextAlignModeEnum = EnumConverterHolder<TextAlignModeEnumConverter>;

void from_json(const nlohmann::json &j, TextAlignMode &mode);

using InnerAlignMode = TextAlignMode;
// using InnerAlignModeEnum = TextAlignModeEnum;

//----------------------------------------------------------------------------------

enum class ImageScaleMode {
    None,
    Center,
    ScaleToFit,
    ScaleProportional,
    ScaleProportionalCenter,
};
// struct ImageAlignModeConverter : Space::EnumConverter<ImageScaleMode, ImageScaleMode::None> {
//     ImageAlignModeConverter() {
//         Add("Default", Enum::kNone);
//         Add("Center", Enum::kCenter);
//         Add("ScaleToFit", Enum::kScaleToFit);
//         Add("ScaleProportional", Enum::kScaleProportional);
//         Add("ScaleProportionalCenter", Enum::kScaleProportionalCenter);
//     }
// };
// using ImageScaleModeEnum = Space::EnumConverterHolder<ImageAlignModeConverter>;

//----------------------------------------------------------------------------------

enum class Orientation {
    Horizontal,
    Vertical,
};
// struct OrientationEnumConverter : Space::EnumConverter<Orientation, Orientation::Horizontal> {
//     OrientationEnumConverter() {
//         Add("Default", Enum::kHorizontal);
//         Add("Horizontal", Enum::kHorizontal);
//         Add("Vertical", Enum::kVertical);
//     }
// };
// using OrientationEnum = Space::EnumConverterHolder<OrientationEnumConverter>;

//----------------------------------------------------------------------------------

enum class ScaleMode {
    None,
    Fit,
    ProportionalFit,
    User,
};
// struct ScaleModeEnumConverter : Space::EnumConverter<ScaleMode, ScaleMode::None> {
//     ScaleModeEnumConverter() {
//         Add("Default", Enum::kNone);
//         Add("None", Enum::kNone);
//         Add("Fit", Enum::kFit);
//         Add("ProportionalFit", Enum::kProportionalFit);
//         Add("User", Enum::kUser);
//     }
// };
// using ScaleModeEnum = Space::EnumConverterHolder<ScaleModeEnumConverter>;

//----------------------------------------------------------------------------------

} // namespace MoonGlare::Component::Rect

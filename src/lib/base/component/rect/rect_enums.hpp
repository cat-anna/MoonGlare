#pragma once

#include <cstdint>

namespace MoonGlare::Component::Rect {

enum class AlignMode : uint8_t {
    None,
    Top,
    Bottom,
    Left,
    Right,
    LeftTop,
    LeftBottom,
    RightTop,
    RightBottom,
    LeftMiddle,
    RightMiddle,
    MiddleTop,
    MiddleBottom,
    FillParent,
    Center,
    Table,
};
// struct AlignModeEnumConverter : Space::EnumConverter<AlignMode, AlignMode::None> {
//     AlignModeEnumConverter() {
//         Add("Default", Enum::None);
//         Add("Parent", Enum::FillParent);
//         Add("FillParent", Enum::FillParent);

//         Add("Center", Enum::Center);
//         Add("Top", Enum::Top);
//         Add("Bottom", Enum::Bottom);
//         Add("Left", Enum::Left);
//         Add("Right", Enum::Right);
//         Add("LeftMiddle", Enum::LeftMiddle);
//         Add("RightMiddle", Enum::RightMiddle);
//         Add("MiddleTop", Enum::MiddleTop);
//         Add("MiddleBottom", Enum::MiddleBottom);

//         Add("LeftTop", Enum::LeftTop);
//         Add("LeftBottom", Enum::LeftBottom);
//         Add("RightTop", Enum::RightTop);
//         Add("RightBottom", Enum::RightBottom);

//         Add("Table", Enum::Table);
//     }
// };
// using AlignModeEnum = Space::EnumConverterHolder<AlignModeEnumConverter>;

//----------------------------------------------------------------------------------

enum class TextAlignMode : uint8_t {
    LeftTop,
    MiddleTop,
    RightTop,
    LeftMiddle,
    Middle,
    RightMiddle,
    LeftBottom,
    MiddleBottom,
    RightBottom,
    //Justified, //:)
};
// struct TextAlignModeEnumConverter : Space::EnumConverter<TextAlignMode, TextAlignMode::LeftTop> {
//     TextAlignModeEnumConverter() {
//         Add("Default", Enum::LeftTop);
//         //Add("Justified", Enum::Justified);
//         Add("LeftTop", Enum::LeftTop);
//         Add("MiddleTop", Enum::MiddleTop);
//         Add("RightTop", Enum::RightTop);
//         Add("LeftMiddle", Enum::LeftMiddle);
//         Add("Middle", Enum::Middle);
//         Add("RightMiddle", Enum::RightMiddle);
//         Add("LeftBottom", Enum::LeftBottom);
//         Add("MiddleBottom", Enum::MiddleBottom);
//         Add("RightBottom", Enum::RightBottom);
//     }
// };
// using TextAlignModeEnum = Space::EnumConverterHolder<TextAlignModeEnumConverter>;

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
//         Add("Default", Enum::None);
//         Add("Center", Enum::Center);
//         Add("ScaleToFit", Enum::ScaleToFit);
//         Add("ScaleProportional", Enum::ScaleProportional);
//         Add("ScaleProportionalCenter", Enum::ScaleProportionalCenter);
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
//         Add("Default", Enum::Horizontal);
//         Add("Horizontal", Enum::Horizontal);
//         Add("Vertical", Enum::Vertical);
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
//         Add("Default", Enum::None);
//         Add("None", Enum::None);
//         Add("Fit", Enum::Fit);
//         Add("ProportionalFit", Enum::ProportionalFit);
//         Add("User", Enum::User);
//     }
// };
// using ScaleModeEnum = Space::EnumConverterHolder<ScaleModeEnumConverter>;

//----------------------------------------------------------------------------------

} // namespace MoonGlare::Component::Rect

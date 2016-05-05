#pragma once

namespace MoonGlare {
namespace GUI {

	enum class AlignMode {
		None, 
		Top, Bottom, Left, Right, 
		//LeftTop, LeftBottom,   //To be implemented...
		//RightTop, RightBottom, //To be implemented...
		LeftMiddle, RightMiddle,
		MiddleTop, MiddleBottom,
		Parent, Center,
	};
	struct AlignModeEnumConverter : Space::EnumConverter < AlignMode, AlignMode::None > {
		AlignModeEnumConverter() {
			Add("Default", Enum::None);
			Add("Parent", Enum::Parent);

			Add("Center", Enum::Center);
			Add("Top", Enum::Top);
			Add("Bottom", Enum::Bottom);
			Add("Left", Enum::Left);
			Add("Right", Enum::Right);
			Add("LeftMiddle", Enum::LeftMiddle);
			Add("RightMiddle", Enum::RightMiddle);
			Add("MiddleTop", Enum::MiddleTop);
			Add("MiddleBottom", Enum::MiddleBottom);
		}
	};
	using AlignModeEnum = Space::EnumConverterHolder < AlignModeEnumConverter > ;

//----------------------------------------------------------------------------------
	
	enum class TextAlignMode {
		LeftTop,		MiddleTop,		RightTop,
		LeftMiddle,		Middle,			RightMiddle,
		LeftBottom,		MiddleBottom,	RightBottom,
		//Justified, //:)
	};
	struct TextAlignModeEnumConverter : Space::EnumConverter < TextAlignMode, TextAlignMode::LeftTop > {
		TextAlignModeEnumConverter() {
			Add("Default", Enum::LeftTop);
			//Add("Justified", Enum::Justified);
			Add("LeftTop", Enum::LeftTop);
			Add("MiddleTop", Enum::MiddleTop);
			Add("RightTop", Enum::RightTop);
			Add("LeftMiddle", Enum::LeftMiddle);
			Add("Middle", Enum::Middle);
			Add("RightMiddle", Enum::RightMiddle);
			Add("LeftBottom", Enum::LeftBottom);
			Add("MiddleBottom", Enum::MiddleBottom);
			Add("RightBottom", Enum::RightBottom);
		}
	};
	using TextAlignModeEnum = Space::EnumConverterHolder < TextAlignModeEnumConverter > ;
	
	using InnerAlignMode = TextAlignMode;
	using InnerAlignModeEnum = TextAlignModeEnum;

//----------------------------------------------------------------------------------

	enum class Orientation {
		Horizontal, Vertical,
	};
	struct OrientationEnumConverter : Space::EnumConverter < Orientation, Orientation::Horizontal > {
		OrientationEnumConverter() {
			Add("Default", Enum::Horizontal);
			Add("Horizontal", Enum::Horizontal);
			Add("Vertical", Enum::Vertical);
		}
	};
	using OrientationEnum = Space::EnumConverterHolder < OrientationEnumConverter > ;

//----------------------------------------------------------------------------------

	enum class ScaleMode {
		None, Fit, ProportionalFit, User,
	};
	struct ScaleModeEnumConverter : Space::EnumConverter < ScaleMode, ScaleMode::None > {
		ScaleModeEnumConverter() {
			Add("Default", Enum::None);
			Add("None", Enum::None);
			Add("Fit", Enum::Fit);
			Add("ProportionalFit", Enum::ProportionalFit);
			Add("User", Enum::User);
		}
	};
	using ScaleModeEnum = Space::EnumConverterHolder < ScaleModeEnumConverter > ;

//----------------------------------------------------------------------------------

} //namespace GUI 
} //namespace MoonGlare 

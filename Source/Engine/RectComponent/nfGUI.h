#pragma once

namespace MoonGlare {
namespace GUI {
	using Point = ::glm::fvec2;
	using UnsignedPoint = ::glm::uvec2;

	class Mouse;
	class Screen;

	struct Font;
	using SharedFont = std::shared_ptr < Font > ;
	using GuiFont = std::shared_ptr < Font > ;

	//class Animation;
	//using SharedAnimation = std::shared_ptr < Animation > ;
	//using WeakAnimation = std::weak_ptr < Animation > ;
	//struct AnimationInstance;

	struct StyleSet;
	using SharedStyleSet = std::shared_ptr < StyleSet > ;
	class StyleManager;

	//using MouseButton = ::Graphic::WindowInput::MouseButton;
	//using ModsStatus = ::Graphic::WindowInput::ModsStatus;

	//namespace Events {
	//	struct Event;

	//	struct MouseEvent;
	//	struct MouseEnterEvent;
	//	struct MouseLeaveEvent;
	//	struct MouseMoveEvent;
	//	struct MouseDownEvent;
	//	struct MouseUpEvent;
	//}
} //namespace GUI 
} //namespace MoonGlare 

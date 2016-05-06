#pragma once

namespace MoonGlare {
namespace GUI {
	class GUIEngine;
	using GUIEnginePtr = std::unique_ptr < GUIEngine > ;

	class iWidget;
	struct WidgetDeleter {
		void operator()(iWidget *w);
	};
	using Widget = std::unique_ptr < iWidget, WidgetDeleter > ;

	class Mouse;
	class Screen;

	struct Font;
	using SharedFont = std::shared_ptr < Font > ;
	using GuiFont = std::shared_ptr < Font > ;

	class Animation;
	using SharedAnimation = std::shared_ptr < Animation > ;
	using WeakAnimation = std::weak_ptr < Animation > ;
	struct AnimationInstance;

	struct StyleSet;
	using SharedStyleSet = std::shared_ptr < StyleSet > ;
	class StyleManager;

	using WidgetClassRegister = Space::DynamicClassRegister < iWidget, iWidget* > ;

	using MouseButton = ::Graphic::WindowInput::MouseButton;
	using ModsStatus = ::Graphic::WindowInput::ModsStatus;

	namespace Widgets {
		class Image;
		class Label;
		class Panel;
		class Table;
	}

	namespace Events {
		struct Event;

		struct MouseEvent;
		struct MouseEnterEvent;
		struct MouseLeaveEvent;
		struct MouseMoveEvent;
		struct MouseDownEvent;
		struct MouseUpEvent;
	}
} //namespace GUI 
} //namespace MoonGlare 

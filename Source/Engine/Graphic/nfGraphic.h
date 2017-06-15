#ifndef NFGRAPHIC_H_
#define NFGRAPHIC_H_

namespace MoonGlare {
namespace Renderer {

struct RenderInput;

}
}

namespace Graphic {

	using ::MoonGlare::Renderer::RenderInput;

	class cRenderDevice;
	class LoadQueue;
	struct MatrixStack;

	class VAO;

	struct StaticFog;
	class Environment;

	struct Material;

	class Window;
	using WindowPtr = std::unique_ptr < Window > ;

	using vec4 = ::math::vec4;
	using vec3 = ::math::vec3;

	using vec2 = ::glm::vec2;
	using ivec2 = ::glm::ivec2;
	using uvec2 = ::glm::uvec2;

	using mat4 = ::glm::mat4;

	using cVertexVector = std::vector<vec3>;
	using VertexVector = std::vector<vec3>;

	using cNormalVector = std::vector<vec3>;
	using NormalVector = std::vector<vec3>;

	using cTexCoordVector = std::vector<vec2>;
	using TexCoordVector = std::vector<vec2>;

	using IndexVector = std::vector<unsigned>;
//	typedef std::vector<unsigned short> cShortIndexVector;

	using QuadArray3 = std::array<math::vec3, 4>;
	using QuadArray2 = std::array<math::vec2, 4>;

	namespace WindowInput {
		enum class Key : unsigned {
			Enter				= GLFW_KEY_ENTER,
			Space				= GLFW_KEY_SPACE,
			Escape				= GLFW_KEY_ESCAPE,
			Backspace			= GLFW_KEY_BACKSPACE,
			Delete				= GLFW_KEY_DELETE,
			ArrowUp				= GLFW_KEY_UP,
			ArrowDown			= GLFW_KEY_DOWN,
			ArrowLeft			= GLFW_KEY_LEFT,
			ArrowRight			= GLFW_KEY_RIGHT,
			Tab					= GLFW_KEY_TAB,
			LeftShift			= GLFW_KEY_LEFT_SHIFT,
			RightShift			= GLFW_KEY_RIGHT_SHIFT,
			GraveAccent			= GLFW_KEY_GRAVE_ACCENT,

			FirstFunctionalKey  = 256,

			CharA				= GLFW_KEY_A,
			CharB				= GLFW_KEY_B,
			CharC				= GLFW_KEY_C,
			CharD				= GLFW_KEY_D,
			CharE				= GLFW_KEY_E,
			CharF				= GLFW_KEY_F,
			CharG				= GLFW_KEY_G,
			CharH				= GLFW_KEY_H,
			CharI				= GLFW_KEY_I,
			CharJ				= GLFW_KEY_J,
			CharK				= GLFW_KEY_K,
			CharL				= GLFW_KEY_L,
			CharM				= GLFW_KEY_M,
			CharN				= GLFW_KEY_N,
			CharO				= GLFW_KEY_O,
			CharP				= GLFW_KEY_P,
			CharQ				= GLFW_KEY_Q,
			CharR				= GLFW_KEY_R,
			CharS				= GLFW_KEY_S,
			CharT				= GLFW_KEY_T,
			CharU				= GLFW_KEY_U,
			CharV				= GLFW_KEY_V,
			CharW				= GLFW_KEY_W,
			CharX				= GLFW_KEY_X,
			CharY				= GLFW_KEY_Y,
			CharZ				= GLFW_KEY_Z,
			Char0				= GLFW_KEY_0,
			Char1				= GLFW_KEY_1,
			Char2				= GLFW_KEY_2,
			Char3				= GLFW_KEY_3,
			Char4				= GLFW_KEY_4,
			Char5				= GLFW_KEY_5,
			Char6				= GLFW_KEY_6,
			Char7				= GLFW_KEY_7,
			Char8				= GLFW_KEY_8,
			Char9				= GLFW_KEY_9,
			KeyF1 				= GLFW_KEY_F1, 
			KeyF2 				= GLFW_KEY_F2, 
			KeyF3 				= GLFW_KEY_F3, 
			KeyF4 				= GLFW_KEY_F4, 
			KeyF5 				= GLFW_KEY_F5, 
			KeyF6 				= GLFW_KEY_F6, 
			KeyF7 				= GLFW_KEY_F7, 
			KeyF8 				= GLFW_KEY_F8, 
			KeyF9 				= GLFW_KEY_F9, 
			KeyF10				= GLFW_KEY_F10,
			KeyF11				= GLFW_KEY_F11,
			KeyF12				= GLFW_KEY_F12,
		};

		enum class MouseButton {
			Left = GLFW_MOUSE_BUTTON_LEFT,
			Right = GLFW_MOUSE_BUTTON_RIGHT,
			Middle = GLFW_MOUSE_BUTTON_MIDDLE,

			Button_1 = GLFW_MOUSE_BUTTON_1,//left 
			Button_2 = GLFW_MOUSE_BUTTON_2,//right
			Button_3 = GLFW_MOUSE_BUTTON_3,//middle
			Button_4 = GLFW_MOUSE_BUTTON_4,
			Button_5 = GLFW_MOUSE_BUTTON_5,
			Button_6 = GLFW_MOUSE_BUTTON_6,
			Button_7 = GLFW_MOUSE_BUTTON_7,
			Button_8 = GLFW_MOUSE_BUTTON_8,
		};	

		enum class ModiffierFlags {
			Shift		= GLFW_MOD_SHIFT,
			Control		= GLFW_MOD_CONTROL,  
			Alt			= GLFW_MOD_ALT,  
			Super		= GLFW_MOD_SUPER,  
		};

		struct ModsStatus {
			unsigned RawValue;

			DefineFlagGetter(RawValue, (unsigned)ModiffierFlags::Shift, ShiftPressed);
			DefineFlagGetter(RawValue, (unsigned)ModiffierFlags::Control, ControlPressed);
			DefineFlagGetter(RawValue, (unsigned)ModiffierFlags::Alt, AltPressed);
			DefineFlagGetter(RawValue, (unsigned)ModiffierFlags::Super, SuperPressed);
		};
	};
}// namespace Graphic

#include "Dereferred/nfDereferred.h"

#endif // GRAPHIC_H_ 

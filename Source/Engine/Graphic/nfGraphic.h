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

	class VirtualCamera;
	using VirtualCameraPtr = std::unique_ptr < VirtualCamera > ;

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

	template <GLenum TEX_MODE> class Texture_template;
	typedef Texture_template<GL_TEXTURE_2D> Texture;
	typedef Texture_template<GL_TEXTURE_3D> Texture3d;
	typedef Texture_template<GL_TEXTURE_RECTANGLE> RectangleTexture;

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

			Char_A				= GLFW_KEY_A,
			Char_B				= GLFW_KEY_B,
			Char_C				= GLFW_KEY_C,
			Char_D				= GLFW_KEY_D,
			Char_E				= GLFW_KEY_E,
			Char_F				= GLFW_KEY_F,
			Char_G				= GLFW_KEY_G,
			Char_H				= GLFW_KEY_H,
			Char_I				= GLFW_KEY_I,
			Char_J				= GLFW_KEY_J,
			Char_K				= GLFW_KEY_K,
			Char_L				= GLFW_KEY_L,
			Char_M				= GLFW_KEY_M,
			Char_N				= GLFW_KEY_N,
			Char_O				= GLFW_KEY_O,
			Char_P				= GLFW_KEY_P,
			Char_Q				= GLFW_KEY_Q,
			Char_R				= GLFW_KEY_R,
			Char_S				= GLFW_KEY_S,
			Char_T				= GLFW_KEY_T,
			Char_U				= GLFW_KEY_U,
			Char_V				= GLFW_KEY_V,
			Char_W				= GLFW_KEY_W,
			Char_X				= GLFW_KEY_X,
			Char_Y				= GLFW_KEY_Y,
			Char_Z				= GLFW_KEY_Z,
			Char_0				= GLFW_KEY_0,
			Char_1				= GLFW_KEY_1,
			Char_2				= GLFW_KEY_2,
			Char_3				= GLFW_KEY_3,
			Char_4				= GLFW_KEY_4,
			Char_5				= GLFW_KEY_5,
			Char_6				= GLFW_KEY_6,
			Char_7				= GLFW_KEY_7,
			Char_8				= GLFW_KEY_8,
			Char_9				= GLFW_KEY_9,
			Key_F1 				= GLFW_KEY_F1, 
			Key_F2 				= GLFW_KEY_F2, 
			Key_F3 				= GLFW_KEY_F3, 
			Key_F4 				= GLFW_KEY_F4, 
			Key_F5 				= GLFW_KEY_F5, 
			Key_F6 				= GLFW_KEY_F6, 
			Key_F7 				= GLFW_KEY_F7, 
			Key_F8 				= GLFW_KEY_F8, 
			Key_F9 				= GLFW_KEY_F9, 
			Key_F10				= GLFW_KEY_F10,
			Key_F11				= GLFW_KEY_F11,
			Key_F12				= GLFW_KEY_F12,
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

#include "Settings.h"
#include "Shaders/nfShaders.h"
#include "Dereferred/nfDereferred.h"

#endif // GRAPHIC_H_ 

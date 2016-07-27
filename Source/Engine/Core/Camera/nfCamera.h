#pragma once 

namespace MoonGlare {
namespace Core {
	namespace Camera {
		class iCamera;
		using iCameraPtr = std::unique_ptr < iCamera > ;
	}
}
} //namespace MoonGlare 

using MoonGlare::Core::Camera::iCameraPtr;

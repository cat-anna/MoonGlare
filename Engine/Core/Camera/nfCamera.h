#pragma once 

namespace Core {
	namespace Camera {
		class iCamera;
		using iCameraPtr = std::unique_ptr < iCamera > ;
	}
}

using ::Core::Camera::iCameraPtr;

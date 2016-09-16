/*
 * cVirtualCamera.h
 *
 *  Created on: 08-11-2013
 *      Author: Paweu
 */

#ifndef VIRTUALCAMERA_H_
#define VIRTUALCAMERA_H_

namespace MoonGlare {
namespace Renderer {

struct VirtualCamera {
	math::mat4 m_ProjectionMatrix;
	math::vec3 m_Position;
	math::vec3 m_Direction;

	const math::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	void GetModelMatrix(const math::mat4 &ModelMatrix, math::mat4 &output){
		output = m_ProjectionMatrix * ModelMatrix;
	}

	void SetPerspective(float Aspect, float FoV = 45.0f, float Near = 0.1f, float Far = 1.0e4f) {
		m_ProjectionMatrix = glm::perspective(glm::radians(FoV), Aspect, Near, Far);
	}
	void SetOrthogonal(float Width, float Height) {
		m_ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
	}
	void SetOrthogonalUniform(float Width, float Height) {
		m_ProjectionMatrix = glm::ortho(-Width, Width, Height, -Height);
	}
	void SetOrthogonalRect(float left, float top, float right, float bottom) {
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top);
	}
	void SetDefaultPerspective(const math::fvec2 &ScreenSize) {
		SetPerspective(ScreenSize[0] / ScreenSize[1]);
	}
	void SetDefaultOrthogonal(const math::fvec2 &ScreenSize) {
		SetOrthogonal(ScreenSize[0], ScreenSize[1]);
	}
	void SetDefaultOrthogonalUniform(const math::fvec2 &ScreenSize) {
		float Aspect = ScreenSize[0] / ScreenSize[1];
		SetOrthogonalUniform(Aspect, 1.0f);
	}
};

} //namespace Renderer 
} //namespace MoonGlare 

#endif // VIRTUALCAMERA_H_

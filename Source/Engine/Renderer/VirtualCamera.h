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
	math::vec3 m_Position;
	math::vec3 m_Direction;
	math::mat4 m_WorldMatrix;
	math::mat4 m_ProjectionMatrix;

	const math::mat4& GetCameraMatrix() const { return m_WorldMatrix; }

	void GetModelMatrix(const math::mat4 &ModelMatrix, math::mat4 &output){
		output = m_WorldMatrix * ModelMatrix;
	}

//	void UpdateMatrix() {
//		if (!m_UseViewMatrix) return;
//		auto ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, math::vec3(0,1,0));//TODO: calculate up vector!
//		m_WorldMatrix = m_ProjectionMatrix * ViewMatrix;
//	}

	void SetPerspective(float Aspect, float FoV = 45.0f, float Near = 0.1f, float Far = 1.0e4f) {
		m_ProjectionMatrix = glm::perspective(glm::radians(FoV), Aspect, Near, Far);
		m_WorldMatrix = m_ProjectionMatrix;
	}

	void SetOrthogonal(float Width, float Height) {
		m_ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
		m_WorldMatrix = m_ProjectionMatrix;
	}

	void SetDefaultPerspective(const math::fvec2 &ScreenSize) {
		SetPerspective(ScreenSize[0] / ScreenSize[1]);
	}
	void SetDefaultOrthogonal(const math::fvec2 &ScreenSize) {
		SetOrthogonal(ScreenSize[0], ScreenSize[1]);
	}
};

} //namespace Renderer 
} //namespace MoonGlare 

#endif // VIRTUALCAMERA_H_

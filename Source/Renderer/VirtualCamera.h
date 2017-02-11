/*
 * cVirtualCamera.h
 *
 *  Created on: 08-11-2013
 *      Author: Paweu
 */
#pragma once

namespace MoonGlare::Renderer {

struct alignas(16) VirtualCamera {
	emath::fmat4 m_ProjectionMatrix;
	emath::fvec3 m_Position;
	emath::fvec3 m_Direction;

	const emath::fmat4& GetProjectionMatrix() const {
		return m_ProjectionMatrix;
	}

	void GetModelMatrix(const emath::fmat4 &ModelMatrix, emath::fmat4 &output){
		output = m_ProjectionMatrix * ModelMatrix;
	}

	void SetPerspective(float Aspect, float FoV = 45.0f, float Near = 0.1f, float Far = 1.0e4f) {
		m_ProjectionMatrix = emath::MathCast<emath::fmat4>(glm::perspective(glm::radians(FoV), Aspect, Near, Far));
	}
	void SetOrthogonal(float Width, float Height) {
		m_ProjectionMatrix = emath::MathCast<emath::fmat4>(glm::ortho(0.0f, Width, Height, 0.0f));
	}
	void SetOrthogonalUniform(float Width, float Height) {
		m_ProjectionMatrix = emath::MathCast<emath::fmat4>(glm::ortho(-Width, Width, Height, -Height));
	}
	void SetOrthogonalRect(float left, float top, float right, float bottom) {
		m_ProjectionMatrix = emath::MathCast<emath::fmat4>(glm::ortho(left, right, bottom, top));
	}
	void SetOrthogonalRect(float left, float top, float right, float bottom, float Near, float Far) {
		m_ProjectionMatrix = emath::MathCast<emath::fmat4>(glm::ortho(left, right, bottom, top, Near, Far));
	}
	void SetDefaultPerspective(const emath::fvec2 &ScreenSize) {
		SetPerspective(ScreenSize[0] / ScreenSize[1]);
	}
	void SetDefaultPerspective(const math::fvec2 &ScreenSize) {
		SetPerspective(ScreenSize[0] / ScreenSize[1]);
	}
	void SetDefaultOrthogonal(const emath::fvec2 &ScreenSize) {
		SetOrthogonal(ScreenSize[0], ScreenSize[1]);
	}
	void SetDefaultOrthogonal(const math::fvec2 &ScreenSize) {
		SetOrthogonal(ScreenSize[0], ScreenSize[1]);
	}
	void SetDefaultOrthogonalUniform(const emath::fvec2 &ScreenSize) {
		float Aspect = ScreenSize[0] / ScreenSize[1];
		SetOrthogonalUniform(Aspect, 1.0f);
	}
	void SetDefaultOrthogonalUniform(const math::fvec2 &ScreenSize) {
		float Aspect = ScreenSize[0] / ScreenSize[1];
		SetOrthogonalUniform(Aspect, 1.0f);
	}
};

} //namespace MoonGlare::Renderer 

/*
 * cVirtualCamera.cpp
 *
 *  Created on: 08-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include "Graphic.h"

namespace Graphic {

GABI_IMPLEMENT_CLASS_NOCREATOR(VirtualCamera)

VirtualCamera::VirtualCamera() :
		BaseClass(),
		m_UseViewMatrix(true),
		m_Position(0, 0, 0),
		m_Direction(-1, 0, 0),
		m_ProjectionMatrix(),
		m_WorldMatrix() {
}
 
VirtualCamera::~VirtualCamera() {
}

//----------------------------------------------------------------------------------

VirtualCameraPtr VirtualCamera::Orthogonal() {
	auto camera = std::make_unique<ThisClass>();
	camera->SetDefaultOrthogonal();
	return camera;
}
VirtualCameraPtr VirtualCamera::Perspective() {
	auto camera = std::make_unique<ThisClass>();
	camera->SetDefaultPerspective();
	return camera;
}

//----------------------------------------------------------------------------------

void VirtualCamera::SetPerspective(float Aspect, float FoV, float Near, float Far) {
	m_ProjectionMatrix = glm::perspective(glm::radians(FoV), Aspect, Near, Far);
	m_UseViewMatrix = true;
	UpdateMatrix();
}

void VirtualCamera::SetOrthogonal(float Width, float Height) {
	m_ProjectionMatrix = glm::ortho(0.0f, Width, Height, 0.0f);
	m_UseViewMatrix = false;
	UpdateMatrix();
	m_WorldMatrix = m_ProjectionMatrix;
}

void VirtualCamera::SetDefaultPerspective() {
	auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
	SetPerspective(ScreenSize[0] / ScreenSize[1]);
}

void VirtualCamera::SetDefaultOrthogonal() {
	auto ScreenSize = math::fvec2(GetRenderDevice()->GetContextSize());
	SetOrthogonal(ScreenSize[0], ScreenSize[1]);
}

} // namespace Core

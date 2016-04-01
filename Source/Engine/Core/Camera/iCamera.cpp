/*
 * cGameCamera.cpp
 *
 *  Created on: 12-01-2014
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
namespace Core {
namespace Camera {

GABI_IMPLEMENT_STATIC_CLASS(iCamera)
inline iCamera* CurrentInstance() { return GetEngine()->GetCurrentScene()->GetCamera().get(); }
RegisterApiInstance(iCamera, &CurrentInstance, "Camera");
RegisterApiDerivedClass(iCamera, &iCamera::RegisterScriptApi);

iCamera::iCamera(GameScene *Scene) :
		BaseClass(),
		m_PointAt(),
		m_TrackedObject(),
		m_Camera(),
		m_CameraDelta(0, 0, 0),
		m_GameScene(Scene) {
	m_Camera = std::make_unique<Graphic::VirtualCamera>();
	m_Camera->SetDefaultPerspective();
}

iCamera::~iCamera() {
}

//---------------------------------------------------------------------------------------

void iCamera::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cCamera")
		.addFunction("SetTrackedObjectByName", &ThisClass::SetTrackedObjectByName)
		.addFunction("SetTrackedObject", &ThisClass::TrackedObject)

		.addFunction("SetCameraDelta", Utils::Template::HiddenVec3Set<ThisClass, Physics::vec3, &ThisClass::m_CameraDelta>::get())
	.endClass()
	;
}

//---------------------------------------------------------------------------------------

void iCamera::Update(const MoveConfig& conf) {
	if (m_TrackedObject) {
		m_Camera->SetPosition(m_TrackedObject->GetPosition() + m_CameraDelta);
		auto dir = m_TrackedObject->GetLookDirection();
	//5	AddLog(Hint, "Direction" << convert(dir));
		m_Camera->SetDirection(dir);
#pragma message ("CAMERA LOOK DIRECTION IS NOT SET")
		m_Camera->UpdateMatrix();
	}
	conf.Camera = m_Camera.get();
}

void iCamera::PointAt(Objects::Object* PointAt) {
	m_PointAt = PointAt;
}

void iCamera::TrackedObject(Objects::Object* TrackedObject) {
	m_TrackedObject = TrackedObject;
}

void iCamera::SetTrackedObjectByName(const string& Name) {
	TrackedObject(m_GameScene->GetObjectByName(Name));
}

} // namespace Camera
} // namespace Core

/*
 * cVirtualCamera.h
 *
 *  Created on: 08-11-2013
 *      Author: Paweu
 */

#ifndef VIRTUALCAMERA_H_
#define VIRTUALCAMERA_H_

namespace Graphic {

class VirtualCamera {
public:
	~VirtualCamera();
	VirtualCamera();

	static VirtualCameraPtr Orthogonal();
	static VirtualCameraPtr Perspective();

	DefineREADAcces(WorldMatrix, mat4)
	DefineREADAcces(ProjectionMatrix, mat4)

	const vec3 &GetPosition() const { return m_Position; }
	const mat4& GetCameraMatrix() const { return m_WorldMatrix; }
	mat4& GetViewMatrix() { return m_ViewMatrix; }

	void GetModelMatrix(const mat4 &ModelMatrix, mat4 &output){
		output = m_WorldMatrix * ModelMatrix;
	}

	void UpdateMatrix() {
		if (!m_UseViewMatrix) return;
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, vec3(0,1,0));//TODO: calculate up vector!
		m_WorldMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void UpdateWorldMatrix() {
		if (!m_UseViewMatrix) return;
		m_WorldMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void SetPosAndDir(const vec3 &NewPos, const vec3 &NewDirec){
		m_Direction = NewDirec;
		m_Position = NewPos;
		return UpdateMatrix();
	}

	void SetPosition(const vec3 &NewPos) { m_Position = NewPos; }
	void SetPosition(const Physics::vec3 &NewPos) { SetPosition(convert(NewPos)); }
	void SetDirection(const vec3 &NewDirec) { m_Direction = NewDirec; }
	void SetDirection(const Physics::vec3 &NewDirec) { SetDirection(convert(NewDirec)); }

	void SetPerspective(float Aspect, float FoV = 45.0f, float Near = 0.1f, float Far = 1.0e4f);
	void SetOrthogonal(float Width, float Height);

	/** Calls SetPerspective with values from Engine Settings */
	void SetDefaultPerspective();
	/** Calls SetOrthogonal with values from Engine Settings */
	void SetDefaultOrthogonal();
private:
	bool m_UseViewMatrix;
	vec3 m_Position;
	vec3 m_Direction;

	mat4 m_ViewMatrix;
	//single projection matrix, to avoid unnecessary recalculation of it.
	mat4 m_ProjectionMatrix;
	//combined projection and view matrix. transforms from model space to camera space.
	mat4 m_WorldMatrix;
};

} // namespace Graphic
#endif // VIRTUALCAMERA_H_

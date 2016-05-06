/*
 * cGameCamera.h
 *
 *  Created on: 12-01-2014
 *      Author: Paweu
 */
#ifndef CGAMECAMERA_H_
#define CGAMECAMERA_H_
namespace Core {
namespace Camera {

class iCamera : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(iCamera, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	iCamera(GameScene *GameScene);
	~iCamera();

	void Update(const MoveConfig& conf) ;

	void PointAt(Objects::Object *PointAt = 0);
	void TrackedObject(Objects::Object *TrackedObject);

	DefineSetGetByRef(CameraDelta, Physics::vec3);

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	GameScene *m_GameScene;
	Objects::Object *m_PointAt, *m_TrackedObject;
	Graphic::VirtualCameraPtr m_Camera;
	Physics::vec3 m_CameraDelta;

	//for scripts use
	//void SetTrackedObjectByName(const string& Name);
};

} // namespace Camera 
} // namespace Core 
#endif 

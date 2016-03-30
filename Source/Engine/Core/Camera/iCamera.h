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

class iCamera : public Objects::Object {
	GABI_DECLARE_STATIC_CLASS(iCamera, Objects::Object)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	iCamera(GameScene *Scene = nullptr);
	~iCamera();

	virtual void DropDead() override;
	 
	virtual void DoMove(const MoveConfig& conf) override;
	virtual void PreRender(const PreRenderConfig& conf) override;

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	void PointAt(Objects::Object *PointAt = 0);
	void TrackedObject(Objects::Object *TrackedObject);

	DefineSetGetByRef(CameraDelta, Physics::vec3);

	//void Bind(cRenderDevice &dev) { dev.Bind(m_Camera.get()); }
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	Objects::Object *m_PointAt, *m_TrackedObject;
	Graphic::VirtualCameraPtr m_Camera;
	Physics::vec3 m_CameraDelta;

	//for scripts use
	void SetTrackedObjectByName(const string& Name);
};

} // namespace Camera 
} // namespace Core 
#endif 

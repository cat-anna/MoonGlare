#ifndef COBJECTH
#define COBJECTH

namespace Core {
namespace Objects {

DECLARE_SCRIPT_EVENT_VECTOR(DynamicObjectScriptEvents, ObjectScriptEvents,
		SCRIPT_EVENT_ADD(),
		SCRIPT_EVENT_REMOVE());

class DynamicObject : public Object {
	GABI_DECLARE_CLASS(DynamicObject, Object);
	DECLARE_SCRIPT_HANDLERS(DynamicObjectScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	DynamicObject(::Core::GameScene *Scene);
	virtual ~DynamicObject();

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	virtual void DoMove(const MoveConfig& conf) override;
	virtual void PreRender(const PreRenderConfig& conf) override;

	virtual bool LoadPattern(const xml_node node) override;
	virtual bool LoadDynamicState(const xml_node node) override;

	virtual int InvokeOnDropDead() override;
	virtual int InvokeOnTimer(int TimerID) override;
	virtual int InvokeOnUserEventA(int param) override;
	virtual int InvokeOnUserEventB(int param) override;
	virtual int InvokeOnUserEventC(int param) override;
	virtual int InvokeOnUserEventD(int param) override;

	DefineRefGetterAll(MoveController, iMoveControllerPtr);

	static void RegisterScriptApi(ApiInitializer &api);
protected:

	void SetMoveController(MoveControllers::iMoveController *ptr);
private:
	iMoveControllerPtr m_MoveController;
};

} //namespace Objects
} //namespace Core

#endif

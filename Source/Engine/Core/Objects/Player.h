#ifndef cFPSPlayerH
#define cFPSPlayerH

namespace Core {
namespace Objects {

DECLARE_SCRIPT_EVENT_VECTOR(PlayerScriptEvents, ObjectScriptEvents,
		SCRIPT_EVENT_ADD(),
		SCRIPT_EVENT_REMOVE());

class Player : public Object {
	GABI_DECLARE_CLASS_DYNAMIC_SINGLETON(Player, Object);
	DECLARE_SCRIPT_HANDLERS(PlayerScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	Player();
	~Player();

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	virtual void DoMove(const MoveConfig& conf) override;
	virtual void PreRender(const PreRenderConfig& conf) override;

	virtual int InvokeOnDropDead() override;
	virtual int InvokeOnTimer(int TimerID) override;
	virtual int InvokeOnUserEventA(int param) override;
	virtual int InvokeOnUserEventB(int param) override;
	virtual int InvokeOnUserEventC(int param) override;
	virtual int InvokeOnUserEventD(int param) override;

	virtual bool LoadPattern(const xml_node node) override;
	virtual bool LoadDynamicState(const xml_node node) override;

	void SetOwnerScene(GameScene *NewOwner) override;

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	::DataClasses::ModelPtr m_PlayerModel;

	virtual void OnBodyConstruction();
};

} //namespace Objects
} //namespace Core

#endif

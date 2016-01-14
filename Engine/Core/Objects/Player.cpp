#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

GABI_IMPLEMENT_CLASS_DYNAMIC_SINGLETON(Player)
IMPLEMENT_SCRIPT_EVENT_VECTOR(PlayerScriptEvents);
RegisterApiDerivedClass(Player, &Player::RegisterScriptApi);
RegisterApiInstance(Player, &Player::Instance, "Player");
Interfaces::AutoCoreClassReg<Player> PlayerReg("BasicPlayer");

Player::Player():
		BaseClass(nullptr),
		m_PlayerModel(0)
{
	SetName("Player");
	SetThisAsInstance();
	SetScriptHandlers(new PlayerScriptEvents());
	m_CollisionMask.Set(Physics::BodyClass::Player);
	m_CollisionMask.Set(Physics::GroupMask::Player);
}

Player::~Player() {
}

//---------------------------------------------------------------------------------------

void Player::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cPlayer")
	.endClass()
	;
}

//---------------------------------------------------------------------------------------

int Player::InvokeOnDropDead(){
	if (IsDead()) return 0;
	SCRIPT_INVOKE(OnDropDead);
}

int Player::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int Player::InvokeOnUserEventA(int param) { SCRIPT_INVOKE(OnUserEventA, param); }
int Player::InvokeOnUserEventB(int param) { SCRIPT_INVOKE(OnUserEventB, param); }
int Player::InvokeOnUserEventC(int param) { SCRIPT_INVOKE(OnUserEventC, param); }
int Player::InvokeOnUserEventD(int param) { SCRIPT_INVOKE(OnUserEventD, param); }

//---------------------------------------------------------------------------------------

bool Player::LoadPattern(const xml_node node) {
	if (!BaseClass::LoadPattern(node))
		return false;

	xml_node Model = node.child("Model");
	if (Model) {
		const char *name = Model.attribute(xmlAttr_Name).as_string();
		if (!name)
			AddLogf(Error, "Predef object '%s' has defined model without name!", GetName().c_str());
		else {
			m_PlayerModel = GetDataMgr()->GetModel(name);
			//m_ModelInstance = GetScene()->GetInstanceManager().CreateInstance(name);
		}
	}

	return true;
}

bool Player::LoadDynamicState(const xml_node node) {
	if (!BaseClass::LoadDynamicState(node))
		return false;
	return true;
}

//---------------------------------------------------------------------------------------

bool Player::Initialize() {
	return BaseClass::Initialize();
}

bool Player::Finalize() {
	return BaseClass::Finalize();
}

void Player::SetOwnerScene(GameScene *NewOwner) {
	BaseClass::SetOwnerScene(NewOwner);
	if (NewOwner) {
		if (m_PlayerModel) {
			m_ModelInstance = NewOwner->GetInstanceManager().CreateInstance(m_PlayerModel);
			m_ModelInstance->Initialize(this);
		}
	} else {
		if (HaveBody())
			GetBody()->SetWorldOwner(nullptr);
		m_ModelInstance.reset();
	}
}

//---------------------------------------------------------------------------------------

void Player::PreRender(const PreRenderConfig& conf) {
	BaseClass::PreRender(conf);
}

void Player::DoMove(const MoveConfig& conf) {
	BaseClass::DoMove(conf);
}

//---------------------------------------------------------------------------------------

void Player::OnBodyConstruction() {
	BaseClass::OnBodyConstruction();
	m_Body->SetAngularFactor(Physics::vec3(0, 1, 0));
}

} //namespace Objects
} //namespace Core

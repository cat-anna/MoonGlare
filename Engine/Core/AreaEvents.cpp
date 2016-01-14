#include <pch.h>
//#include <MoonGlare.h>
#if 0
#include <GabiLib/D2Math.h>

IMPLEMENT_CLASS(cBasicEventArea);

cBasicEventArea::cBasicEventArea(cFPSEngine *Engine):cObject(){
	m_Flags = 0;
}

//------------------cDamageZone-----------------------------------------------

cDamageZone::cDamageZone():cBasicEventArea(){
	m_Damages = 0;	
}

cDamageZone::cDamageZone(cFPSEngine *Engine):cBasicEventArea(Engine){
	m_Damages = 0;
}

cDamageZone::~cDamageZone(){
	delete m_Damages;
}

int cDamageZone::InvokeOnCollision(cSprite *Spr, bool IsObj){
//	if(IsObj){
//		CollisionList.push_back(dynamic_cast<cObject*>(Spr));
//	}
	return cObject::InvokeOnCollision(Spr, IsObj);
}

void cDamageZone::Init(cObject *Source, cDamages *Damages){
	m_Source = Source;
	m_Damages = Damages;
	m_Position = Source->GetPosition();
}

//-----------------------------------------------------------------

cKillZone::cKillZone():cDamageZone(){
}

cKillZone::cKillZone(cFPSEngine *Engine):cDamageZone(Engine){
}

cKillZone::~cKillZone(){
}

void cKillZone::Init(float Radius, cDamages *TotalDamages, unsigned Count, unsigned TimeStep, cObject *Source){
	cDamageZone::Init(Source, TotalDamages);
	if(m_CollisionArea)delete m_CollisionArea;
	m_Radius = Radius;
	m_CollisionArea = new cCircleArea(m_Radius, m_Position);
	m_TimeStep = TimeStep;
	m_CountRemain = Count;
	m_DmgModifier = m_Damages->GetDmgModifier();
} 

void cKillZone::DoMove(){

}

int cKillZone::InvokeOnCreate(bool InGame){
//	if(m_CountRemain > 0)
//		m_Engine->SetTimer(1, m_TimeStep, this);
	return cObject::InvokeOnCreate(InGame);
}

int cKillZone::InvokeOnTimer(int TimerID){
	/*if(TimerID == 1) {
		CollisionList.clear();
		m_Engine->CheckCollision(this);
		for(cObjectList::iterator i = CollisionList.begin(), j = CollisionList.end(); i != j; ++i){
			cObject *obj = *i;
			float mod = (m_Radius - obj->GetPosition().DistanceXZ(GetPosition()));
			if(mod <= 0) continue;
			mod /= m_Radius;
			m_Damages->SetDmgModifier(m_DmgModifier * mod);
			obj->DoHit(this, m_Damages);
		}

		--m_CountRemain;
		if(m_CountRemain > 0)
			return 1;

		DoDead();
		return 0;		
	}
	else */
	   return cObject::InvokeOnTimer(TimerID);
}

//------------------------cNearestAttackZone-----------------------------------------

cNearestAttackZone::cNearestAttackZone():cDamageZone(){
}

cNearestAttackZone::cNearestAttackZone(cFPSEngine *Engine):cDamageZone(Engine){
}

cNearestAttackZone::~cNearestAttackZone(){
}

void cNearestAttackZone::DoMove(){
	/*if(m_Flags & SPRITE_FLAG_DEAD)return;

	CollisionList.clear();
	m_Engine->CheckCollision(this);
	if(CollisionList.size() > 0){
		cObject* o = *CollisionList.begin();
		float dst = o->GetPosition().DistanceXZ(m_Position);
		for(cObjectList::iterator i = ++CollisionList.begin(), j = CollisionList.end(); i != j; ++i){
//			cObject *obj = *i;
			float d = (*i)->GetPosition().DistanceXZ(m_Position);
			if(d < dst){
				dst = d;
				o = *i;
			}
		}
		o->DoHit(this, m_Damages);
	}
	DoDead();*/
}

void cNearestAttackZone::Init(float Radius, cDamages *Damages, cObject *Source, float AttackAngle, float Distance){
	cDamageZone::Init(Source, Damages);
	if(m_CollisionArea) delete m_CollisionArea;
	m_Radius = Radius;
	GabiLib::D2Math::PointOnCircle(m_Source->GetPosition(), GabiLib::D2Math::DegToRad(AttackAngle), Distance, m_Position);
	m_CollisionArea = new cCircleArea(m_Radius, m_Position);
//	m_DrawPosition.LeftTop.y = m_DrawPosition.RightTop.y = m_Size.y;
} 

//---------------------------------cScriptEventArea-------------------------------------------------------------

IMPLEMENT_CLASS(cScriptEventArea);
IMPLEMENT_SCRIPT_EVENT_VECTOR(cAreaScriptEvents,
		SCRIPT_ADD(OnPlayerEnter)
		SCRIPT_ADD(OnPlayerLeave)
		SCRIPT_NO_INHERIT(OnDead)
		SCRIPT_NO_INHERIT(OnHit)
		SCRIPT_NO_INHERIT(OnAnimationEnds)
		SCRIPT_NO_INHERIT(OnCollision));

cScriptEventArea::cScriptEventArea(cFPSEngine *Engine) : cBasicEventArea(Engine), m_ScriptHandlers(0){
	SetScriptHandlers(new cAreaScriptEvents());
	m_Flags |= BASICOBJECT_SCRIPT_EVENT;
}
		
void cScriptEventArea::DoMove(){
	/*bool Collision = false;
	if(GetCollisionArea())
		Collision = GetCollisionArea()->CheckCollision(m_Engine->GetPlayer()->GetCollisionArea());

	if(Collision) {
		if(m_Flags & sea_CollisionWithPlayer){
			//?
		} else {
			InvokeOnPlayerEnter();
			m_Flags |= sea_CollisionWithPlayer;
		}
	} else {
		if(m_Flags & sea_CollisionWithPlayer){
			InvokeOnPlayerLeave();
			m_Flags &= ~sea_CollisionWithPlayer;
		}
	}*/
}

int cScriptEventArea::InvokeOnPlayerEnter() { 
	INVOKE_FUNCTION_noparam(OnPlayerEnter); 
	return 0;
};

int cScriptEventArea::InvokeOnPlayerLeave() { 
	INVOKE_FUNCTION_noparam(OnPlayerLeave);
	return 0;
};

bool cScriptEventArea::SaveToXML(xml_node Node, bool OnlyPosition){
	m_Position.SaveToXML(Node, 0);
	Node.append_attribute("Name") = m_Name.c_str();
	m_ScriptHandlers->SaveToXML(Node);
	if(m_CollisionArea)
		m_CollisionArea->SaveToXML(Node.append_child("CollisionArea"));
	return true;
}

bool cScriptEventArea::LoadFromXML(xml_node Node){
	m_Position.LoadFromXML(Node);
	m_Name = Node.attribute("Name").as_string();
	m_ScriptHandlers->LoadFromXML(Node);
	if(m_CollisionArea) delete m_CollisionArea;
	m_CollisionArea = cAreaCollision::LoadFromXML(Node.child("CollisionArea"), m_Position);
	return true;
}

bool cScriptEventArea::SaveToPattern(xml_node Node){
	return false;
}

bool cScriptEventArea::LoadFromPattern(xml_node Node){
	return false;
}

#endif

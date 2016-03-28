#if 0
//#ifndef AreaEventsH
#define AreaEventsH

class cBasicEventArea : public cObject {
	DECLARE_CLASS(cBasicEventArea, cObject)
public:
	cBasicEventArea(cFPSEngine *Engine = 0);
//	virtual bool SaveToXML(xml_node Node, bool OnlyPosition = true);
//	virtual bool LoadFromXML(xml_node Node);
};

//--------------------------Damage Events Definitions---------------------------------------

class cDamageZone : public cBasicEventArea {
protected:
	Core::cObjectList CollisionList;
	cObject *m_Source;
	cDamages *m_Damages;
	virtual int InvokeOnCollision(cSprite *Spr, bool IsObj);

	void Init(cObject *Source, cDamages *Damages);
public:
	cDamageZone();
	cDamageZone(cFPSEngine *Engine);
	~cDamageZone();
};

class cKillZone : public cDamageZone {
protected:
	float m_Radius, m_DmgModifier;
	unsigned m_CountRemain, m_TimeStep;
	virtual void DoMove();
	virtual int InvokeOnTimer(int TimerID);
	virtual int InvokeOnCreate(bool InGame);
public:
	cKillZone();
	cKillZone(cFPSEngine *Engine);
	virtual ~cKillZone();

	void Init(float Radius, cDamages *TotalDamages, unsigned Count, unsigned TimeStep, cObject *Source);
//	virtual bool SaveToXML(xml_node Node, bool OnlyPosition = true);
//	virtual bool LoadFromXML(xml_node Node);
};

class cNearestAttackZone : public cDamageZone {
protected:
	float m_Radius;
	virtual void DoMove();
public:
	cNearestAttackZone();
	cNearestAttackZone(cFPSEngine *Engine); 
	virtual ~cNearestAttackZone();
	void Init(float Radius, cDamages *Damages, cObject *Source, float AttackAngle, float Distance);
};

//--------------------------Script Events Definitions---------------------------------------

class cAreaScriptEvents : public cObjectScriptEvents {
	DECLARE_SCRIPT_EVENT_VECTOR(cAreaScriptEvents, cObjectScriptEvents);
public:
	string OnPlayerEnter, OnPlayerLeave;
};

class cScriptEventArea : public cBasicEventArea {
	DECLARE_CLASS(cScriptEventArea, cBasicEventArea)
protected:
	enum{
		sea_CollisionWithPlayer		= 0x10000000,
	};

	DECLARE_SCRIPT_HANDLERS(cAreaScriptEvents);
	virtual int InvokeOnPlayerEnter();
	virtual int InvokeOnPlayerLeave();
public:
	cScriptEventArea(cFPSEngine *Engine = 0);
	virtual void DoMove(); 
	virtual bool SaveToXML(xml_node Node, bool OnlyPosition = true);
	virtual bool LoadFromXML(xml_node Node);
	virtual bool SaveToPattern(xml_node Node);
	virtual bool LoadFromPattern(xml_node Node);
};

#endif

#ifndef GameEngineH
#define GameEngineH
#if 0
namespace Core {

enum {
	MaxStatisticsCount = 30,
	MaxDamageTypesCount = 10,
};

class cValues{
public:
	float value;
	float value_max;

	void Set(float v = 0, float v_max = 0){
		value = v; 
		value_max = v_max;
	}

	bool iszero() const {
		return value == 0 && value_max == 0;
	}
};

class cObjectStatistics : public cRootClass{
friend class cGameEngine;
	cValues m_StatTable[MaxStatisticsCount];
public:
	cObjectStatistics();
	~cObjectStatistics();

	cValues &Acces(unsigned index);//tymczas

	bool SaveToXML(xml_node Parent) const;
	bool LoadFromXML(xml_node Parent);
};

class cDamages : public cRootClass {
friend class cGameEngine;
	cValues m_DmgTable[MaxDamageTypesCount];
	int m_AttackType;
	float m_DmgModifier;
public:
	cDamages();
	~cDamages();

	DefineFULLAcces(AttackType, int);
	DefineFULLAcces(DmgModifier, float);
	cValues &Acces(unsigned index);//tymczas

	bool SaveToXML(xml_node Parent) const;
	bool LoadFromXML(xml_node Parent);	

	void Divide(float value);
};
#if 0
class cGameEngineScripts : public cScriptEvents {
	DECLARE_SCRIPT_EVENT_VECTOR(cGameEngineScripts, cScriptEvents);
public:
	string HitTest, AcquireDamage, Initialize;
};
#endif
class cGameEngine : public cRootClass {
	GABI_DECLARE_CLASS_SINGLETON(cGameEngine, cRootClass)
private:
	unsigned m_Flags;
public:
	enum eStatisticType {
		stNone,			//pusta komórka
		stMaxVal,		//zwyk³a liczbowa wartosæ z maksymaln¹
		stBaseVal,		//						  z bazow¹
		stStringID,		//wartoœæ tekstowa, predefiniowana,
	};
	class cStatisticInfo {
		string m_Name;
		string m_Caption;
		unsigned m_Type;
	public:
		cStatisticInfo();
		DefineFULLAcces(Name, string);
		DefineFULLAcces(Caption, string);
		DefineFULLAcces(Type, unsigned);
		bool SaveToXML(xml_node Node) const;
		bool LoadFromXML(xml_node Node);
		bool Empty() const;
		void Clear();
	};
	class cDamageInfo {
		string m_Name;
		string m_Caption;
	public:
		cDamageInfo();
		DefineFULLAcces(Name, string);
		DefineFULLAcces(Caption, string);
		bool SaveToXML(xml_node Node) const;
		bool LoadFromXML(xml_node Node);
		bool Empty() const;
		void Clear();
	};
protected:
	Objects::Player *m_Player;
	string m_Name;
	//DECLARE_SCRIPT_HANDLERS_ROOT(cGameEngineScripts);
	cStatisticInfo m_StatInfoTable[MaxStatisticsCount];
	cDamageInfo m_DmgInfoTable[MaxDamageTypesCount];

	int InvokeInitialize();
	DefineFlagSetter(m_Flags, ge_Ready, Ready)
public:
	cGameEngine(DataModule *DataModule = 0);
	~cGameEngine();

	void SetDataModule(DataModule *DataModule);

	virtual bool SaveToXML(xml_node Node) const;
	virtual bool LoadFromXML(xml_node Node);

	virtual bool Initialize();
	virtual bool Finalize();

	void test();

//does not require loaded scripts into engine, defines the values depend on the engine
	//bool InitializeScripts(ApiDefInitializer root);

	virtual int InvokeHitTest(Objects::Object *Attacker, Objects::Object *Defender, cDamages *Damage);
	virtual int InvokeAcquireDamage(Objects::Object *Recipient, cDamages *Damage);
	virtual bool PerformHit(Objects::Object *Attacker, Objects::Object *Defender, cDamages *Damage);

	DefineREADAcces(Name, string);
	DefineREADAccesPTR(Player, Objects::Player);

	enum {
		ge_Ready	= 0x0001,
	};
	DefineFlagChecker(m_Flags, ge_Ready, Ready)
};

} //namespace Core
#endif
#endif

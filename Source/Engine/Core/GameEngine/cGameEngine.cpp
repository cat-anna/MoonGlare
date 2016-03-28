#include <pch.h>
#if 0
#include <MoonGlare.h>

namespace Core {

#define xmlGameEngineNode_Options		"Options"
#define xmlGameEngineNode_Player		"Player"

GABI_IMPLEMENT_CLASS_SINGLETON(cGameEngine);

#if 0
IMPLEMENT_SCRIPT_EVENT_VECTOR(cGameEngineScripts,
			SCRIPT_ADD(HitTest)	SCRIPT_ADD(AcquireDamage) SCRIPT_ADD(Initialize));
#endif

cGameEngine::cGameEngine(DataModule *DataModule):
		BaseClass(), 
		m_Flags(0)
		//m_ScriptHandlers() 
{
	SetThisAsInstance();
	//SetScriptHandlers(new cGameEngineScripts());
	m_Player = 0;
}

cGameEngine::~cGameEngine(){
	delete m_Player;
	//delete m_ScriptHandlers;
}

void cGameEngine::SetDataModule(DataModule *DataModule){
//	BaseClass::SetDataModule(DataModule);
}

int cGameEngine::InvokeHitTest(Objects::Object *Attacker, Objects::Object *Defender, cDamages *Damage){
	INVOKE_FUNCTION(HitTest, Attacker, Defender, Damage);
	return 0;
}

int cGameEngine::InvokeAcquireDamage(Objects::Object *Recipient, cDamages *Damage){
	INVOKE_FUNCTION(AcquireDamage, Recipient, Damage);
	return 0;
}

bool cGameEngine::PerformHit(Objects::Object *Attacker, Objects::Object *Defender, cDamages *Damage){
	switch(InvokeHitTest(Attacker, Defender, Damage)){
	case 0: //miss
		AddLog(Hint, "miss");
		LOG_NOT_IMPLEMENTED; 
		return false;
	case 1://hit
		return InvokeAcquireDamage(Defender, Damage) > 0;
	}
	return false;
}

bool cGameEngine::SaveToXML(xml_node Node) const{
	xml_node Options = Node.append_child("Options");
	Options.append_attribute("Name") = m_Name.c_str();

	xml_node subNode = Node.append_child("Statistics");
	for(unsigned i = 0; i < MaxStatisticsCount; ++i){
		if(m_StatInfoTable[i].Empty()) continue;
		xml_node stat = subNode.append_child("Statistic");
		stat.append_attribute("Index") = i;
		m_StatInfoTable[i].SaveToXML(stat);
	}

	subNode = Node.append_child("DamageTypes");
	for(unsigned i = 0; i < MaxDamageTypesCount; ++i){
		if(m_DmgInfoTable[i].Empty()) continue;
		xml_node dmg = subNode.append_child("Damage");
		dmg.append_attribute("Index") = i;
		m_DmgInfoTable[i].SaveToXML(dmg);
	}
	//m_ScriptHandlers->SaveToXML(Node);

	subNode = Node.append_child("Player");
//	GetPlayer()->SaveToPattern(subNode);
	return true;
}

bool cGameEngine::LoadFromXML(xml_node Node) {
	if (IsReady()) {
		AddLog(Error, "Game engine cannot be used again. Create next instance!");
		return false;
	}

	xml_node Options = Node.child(xmlGameEngineNode_Options);
	if(Options){
		m_Name = Options.attribute(xmlAttr_Name).as_string("");
	}
#if 0
	xml_node subNode = Node.child("Statistics");
	if(subNode){
		for(xml_node stat = subNode.child("Statistic"); stat; stat = stat.next_sibling("Statistic")) {
			unsigned idx = stat.attribute("Index").as_uint(MaxStatisticsCount);
			if(idx >= MaxStatisticsCount){
				AddLog(lError << "Statistic index out of range [value: " << idx << "]");
				continue;
			}
			if(m_StatInfoTable[idx].GetType() != stNone){
				AddLog(lError << "Attempt to overwrite loaded statistic! [index: " << idx << "]");
				continue;
			}
			m_StatInfoTable[idx].LoadFromXML(stat);
		}
	}

	subNode = Node.child("DamageTypes");
	if(subNode){
		for(xml_node dmg = subNode.child("Damage"); dmg; dmg = dmg.next_sibling("Damage")) {
			unsigned idx = dmg.attribute("Index").as_uint(MaxDamageTypesCount);
			if(idx >= MaxDamageTypesCount){
				AddLog(lError << "Damage type index out of range [value: " << idx << "]");
				continue;
			}
			m_DmgInfoTable[idx].LoadFromXML(dmg);
		}
	}
	m_ScriptHandlers->LoadFromXML(Node);
#endif

	xml_node PlayerNode = Node.child(xmlGameEngineNode_Player);
	string PlayerClass = PlayerNode.attribute(xmlAttr_Class).as_string(Objects::Player::GetStaticTypeInfo()->Name);
	m_Player = Core::Interfaces::CreateCoreClass<Objects::Player>(PlayerClass);
	CriticalCheck(m_Player, "Unable to create player class!");
	//m_Player->LoadFromPattern(PlayerNode);

	return true;
}

int cGameEngine::InvokeInitialize(){
	INVOKE_FUNCTION_noparam(Initialize);
	return 0;//dummy return
}

bool cGameEngine::Initialize(){
//	if(IsReady()) return true;
//	SetInitialised(InvokeInitialize() == 0);
//	return IsReady();
	LOG_NOT_IMPLEMENTED;
	return true;
}

bool cGameEngine::Finalize() {
	//nothing
	//Game engine cannot be finalized, cleared and loaded again
	//new instance must be created
	return false;
}

void cGameEngine::test(){
	AddLog(Hint, __FUNCTION__ << "  " << std::hex << this);
}

int testint(int v){
	return v;
}

cDamages *AllocDamages(){
	return new cDamages();
}

#if 0
bool cGameEngine::InitializeScripts(ApiDefInitializer root){
	// AddLog(__FUNCTION__ << "   " << std::hex << this);
	LOG_NOT_IMPLEMENTED
	//ndef _USE_DUMMY_SCRIPT_ENGINE_
	unsigned table_pos;
	auto cl_stat = root
			.beginNamespace("api")
			.deriveClass<cObjectStatistics, cRootClass>("cObjectStatistics");

	table_pos = UnionCast<unsigned>(&cObjectStatistics::m_StatTable);
	for(unsigned i = 0; i < MaxStatisticsCount; ++i){
		cStatisticInfo &statinfo = m_StatInfoTable[i];
		unsigned ipos = table_pos + sizeof(cValues) * i;
		switch(statinfo.GetType()){
		case stNone: continue;
		case stMaxVal:{
			cl_stat.addData(statinfo.GetName().c_str(), UnionCast<float cObjectStatistics::*>(ipos));
			string sm = statinfo.GetName() + "_max";
			cl_stat.addData(sm.c_str(), UnionCast<float cObjectStatistics::*>(ipos + sizeof(float)));
			break;
		}
		case stBaseVal:{
			cl_stat.addData(statinfo.GetName().c_str(), UnionCast<float cObjectStatistics::*>(ipos));
			string sm = statinfo.GetName() + "_base";
			cl_stat.addData(sm.c_str(), UnionCast<float cObjectStatistics::*>(ipos + sizeof(float)));
			break;
		}
		case stStringID:
			break;
		}
	}

	auto cl_dmg = cl_stat
			.endClass()
			.addFunction("AllocDamages", &AllocDamages)
			.addFunction("testint", &testint)
			.deriveClass<cDamages, cRootClass>("cDamages")
				.addConstructor<void(*)()>()
				.addData("AttackType", &cDamages::m_AttackType)
				.addData("DmgModifier", &cDamages::m_DmgModifier);

	table_pos = UnionCast<unsigned>(&cDamages::m_DmgTable);
	for(unsigned i = 0; i < MaxDamageTypesCount; ++i){
		cDamageInfo &DmgInfo = m_DmgInfoTable[i];
		if(DmgInfo.Empty()) continue;
		unsigned ipos = table_pos + sizeof(cValues) * i;
		string n_min = DmgInfo.GetName() + "_min";
		string n_max = DmgInfo.GetName() + "_max";
		cl_dmg.addData(n_min.c_str(), UnionCast<float cDamages::*>(ipos));
		cl_dmg.addData(n_max.c_str(), UnionCast<float cDamages::*>(ipos + sizeof(float)));
	}
	auto cont = cl_dmg.endClass();
	return true;
}
#endif

//==============================cStatisticInfo=======================================

cGameEngine::cStatisticInfo::cStatisticInfo():m_Name(), m_Caption(), m_Type(stNone){

}

bool cGameEngine::cStatisticInfo::SaveToXML(xml_node Node) const{
	Node.append_attribute("Type") = m_Type;
	if(m_Type == stNone) return true;
	Node.append_attribute("Name") = m_Name.c_str();
	Node.append_attribute("Caption") = m_Caption.c_str();
	return true;
}

bool cGameEngine::cStatisticInfo::LoadFromXML(xml_node Node){
	m_Type = Node.attribute("Type").as_uint(stNone);
	m_Name = Node.attribute("Name").as_string();
	m_Caption = Node.attribute("Caption").as_string();
	return true;
}

bool cGameEngine::cStatisticInfo::Empty() const{
	return m_Type == stNone || m_Name.empty() || m_Caption.empty();
}

void cGameEngine::cStatisticInfo::Clear(){
	m_Name.clear();
	m_Caption.clear();
	m_Type = stNone;
}

//==============================cDamageInfo=======================================

cGameEngine::cDamageInfo::cDamageInfo():m_Name(), m_Caption(){

}

bool cGameEngine::cDamageInfo::SaveToXML(xml_node Node) const{
	Node.append_attribute("Name") = m_Name.c_str();
	Node.append_attribute("Caption") = m_Caption.c_str();
	return true;
}

bool cGameEngine::cDamageInfo::LoadFromXML(xml_node Node){
	m_Name = Node.attribute("Name").as_string();
	m_Caption = Node.attribute("Caption").as_string();
	return true;
}

bool cGameEngine::cDamageInfo::Empty() const{
	return m_Name.empty() || m_Caption.empty();
}

void cGameEngine::cDamageInfo::Clear(){
	m_Name.clear();
	m_Caption.clear();
}

//==============================cObjectStatistics=======================================

cObjectStatistics::cObjectStatistics(){
	for(unsigned i = 0; i < MaxStatisticsCount; ++i){
		cValues &sv = m_StatTable[i];
		sv.Set();
	}
}

cObjectStatistics::~cObjectStatistics(){

}

cValues &cObjectStatistics::Acces(unsigned index) {
	if(index < MaxStatisticsCount) return m_StatTable[index];
	throw (string(__FUNCTION__) += "  ERROR!");
}

bool cObjectStatistics::SaveToXML(xml_node Parent) const{
	xml_node Statistics;
	for(unsigned i = 0; i < MaxStatisticsCount; ++i){
		const cValues &sv = m_StatTable[i];
		if(sv.value == 0 && sv.value_max == 0)continue;
		if(!Statistics) Statistics = Parent.append_child("Statistics");
		xml_node Stat = Statistics.append_child("Stat"); 
		Stat.append_attribute("Index") = i;
		Stat.append_attribute("Value") = sv.value;
		Stat.append_attribute("Max") = sv.value_max;
	}
	return true;
}

bool cObjectStatistics::LoadFromXML(xml_node Parent){
	xml_node Statistics = Parent.child("Statistics");
	if(!Statistics)return true;

	for(xml_node stat = Statistics.child("Stat"); stat; stat = stat.next_sibling("Stat")) {
		unsigned idx = stat.attribute("Index").as_uint(MaxStatisticsCount);
		if(idx >= MaxStatisticsCount){
			AddLog(Error, "Statistic index out of range [value: " << idx << "]");
			continue;
		}
		cValues &sv = m_StatTable[idx];
		sv.value = stat.attribute("Value").as_float();
		sv.value_max = stat.attribute("Max").as_float();
	}
	return true;
}

//==============================cDamages=======================================

cDamages::cDamages(){
	for(unsigned i = 0; i < MaxDamageTypesCount; ++i)
		m_DmgTable[i].Set();
	m_DmgModifier = 1;
	m_AttackType = 0;
}

cDamages::~cDamages(){

}

cValues &cDamages::Acces(unsigned index) {
	if(index < MaxDamageTypesCount) return m_DmgTable[index];
	throw  (string(__FUNCTION__)+= "  ERROR!");
}

bool cDamages::SaveToXML(xml_node Parent) const{
	xml_node Node;
	for(unsigned i = 0; i < MaxDamageTypesCount; ++i){
		const cValues &sv = m_DmgTable[i];
		if(sv.iszero())continue;
		if(!Node) Node = Parent.append_child("Damages");
		xml_node dmg = Node.append_child("Damage"); 
		dmg.append_attribute("Index") = i;
		dmg.append_attribute("Value") = sv.value;
		dmg.append_attribute("Max") = sv.value_max;
	}
	if(m_AttackType != 0) CheckXML(Parent, "Damages", Node).append_attribute("AttackType") = m_AttackType;
	if(m_DmgModifier != 0) CheckXML(Parent, "Damages", Node).append_attribute("DmgModifier") = m_DmgModifier;
	return true;
}

bool cDamages::LoadFromXML(xml_node Parent){
	xml_node Node = Parent.child("Statistics");
	if(!Node)return true;

	for(xml_node dmg = Node.child("Damage"); dmg; dmg = dmg.next_sibling("Damage")) {
		unsigned idx = dmg.attribute("Index").as_uint(MaxDamageTypesCount);
		if(idx >= MaxDamageTypesCount){
			AddLog(Error, "Damage index out of range [value: " << idx << "]");
			continue;
		}
		cValues &sv = m_DmgTable[idx];
		sv.value = dmg.attribute("Value").as_float();
		sv.value_max = dmg.attribute("Max").as_float();
	}
	m_AttackType = Node.attribute("AttackType").as_int(0);
	m_DmgModifier = Node.attribute("DmgModifier").as_float(1);
	return true;
}

void cDamages::Divide(float value){
	if(value <= 0){
		AddLog(Error, "Dividing damages by zero or less value!");
		return;
	}

	for(unsigned i = 0; i < MaxDamageTypesCount; ++i){
		m_DmgTable[i].value /= value;
		m_DmgTable[i].value_max /= value;
	}
}

} //namespace Core
#endif

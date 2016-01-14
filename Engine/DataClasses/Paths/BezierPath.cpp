/*
	Generated by cppsrc.sh
	On 2015-02-04 22:26:26,44
	by Paweu
*/
#include <pch.h>

namespace DataClasses {
namespace Paths {

#if 0

const sFlagStruct cPathFlagPack[] = {
	{"Circled", cPath::pfCircled, false},
	{0, 0, false},
};

bool cPath::SaveToXML(xml_node Node) const {
	Node.append_attribute("Name") = GetName().c_str();
	Node.append_attribute("NodeCount") = m_NodeVector.size();
///	xml_node Flags = ;
//	if(m_Flags & pfCircled)	Flags.append_attribute("Circled") = true;
	WriteFlagPack(Node.append_child("Flags"), m_Flags, cPathFlagPack);
	for(cPathNodeVector::const_iterator it = m_NodeVector.begin(), jt = m_NodeVector.end(); it != jt; ++it){
		cPathNodeVector::const_iterator next = it + 1;
		if(next == m_NodeVector.end()) next = m_NodeVector.begin();

#if 0
		{
			const c3DPoint &M1 = it->GetMasterPoint();
			//			const c3DPoint &P1 = it->GetSecondPoint();
			//			const c3DPoint &P2 = next->GetFirstPoint();
			//			const c3DPoint &M2 = next->GetMasterPoint();

			float len = 0;
			float step = 1 / 1000.0f;
			c3DPoint b = M1, bc;
			for (float x = step; x <= 1.0f; x += step){
				//				GabiLib::D2Math::BezierPoint(M1, P1, P2, M2, x, bc);
				//				len += bc.DistanceXZ(b);
				b = bc;
			}
			((cPathNode&)*it).SetSectionLen(len);
		}
#endif // 0


		it->SaveToXML(Node.append_child("Node"));
	}
	return true;
}

bool cPath::LoadFromXML(xml_node Node) {
	//m_Name = Node.attribute("Name").as_string("");
	m_NodeVector.resize(Node.attribute("NodeCount").as_uint(0));
//	xml_node Flags = ;
//	m_Flags = 0;
//	if(Flags.attribute("Circled").as_bool(false)) m_Flags |= pfCircled;
	ReadFlagPack(Node.child("Flags"), m_Flags, cPathFlagPack);
	unsigned i = 0;
	for(xml_node it = Node.child("Node"); it; it = it.next_sibling("Node"), ++i){
		if(i >= m_NodeVector.size()) m_NodeVector.push_back(cPathNode());
		m_NodeVector[i].LoadFromXML(it);
//		AddLogf(lError, "%f", m_NodeVector[i].GetSectionLen());
	}
	return true;
}

bool cPath::Initialize() {
	return SetInitialised(true);
}

void cPath::Finalize() {
	SetInitialised(false);
}

math::vec3 cPath::CalcPoint(float position) const {
	if (m_NodeVector.empty()) return math::vec3();
	if(position <= 0) return m_NodeVector[0].GetMasterPoint();
	int len = m_NodeVector.size();
	if(m_Flags & pfCircled){
		if(position >= len)
			position -= len;
	} else {
		len -= 1;
		if(len <= position)
			return m_NodeVector.back().GetMasterPoint();
	}

	int node1 = static_cast<int>(position);
	int node2 = node1 + 1;
	if(node2 == len)node2 = 0;
	//const c3DPoint &M1, const c3DPoint &P1, const c3DPoint &P2, const c3DPoint &M2, float t, c3DPoint &output
	//c3DPoint res;
//	float pos = position - node1;
//	const cPathNode &n1 = m_NodeVector[node1], &n2 = m_NodeVector[node2];
//	return GabiLib::D2Math::BezierPoint(n1.GetMasterPoint(), n1.GetSecondPoint(), n2.GetFirstPoint(), n2.GetMasterPoint(), pos, res);
	return math::vec3();
}

//-------------------------------------------------------------------------------------------------

bool cPathNode::SaveToXML(xml_node Node) const{
//	m_MasterPoint.SaveToXML(Node);
//	m_FirstPoint.SaveToXML(Node.append_child("First"));
//	m_SecondPoint.SaveToXML(Node.append_child("Second"));
	Node.append_attribute("Length") = m_SectionLen;
	return true;
}

bool cPathNode::LoadFromXML(xml_node Node) {
//	m_MasterPoint.LoadFromXML(Node);
//	m_FirstPoint.LoadFromXML(Node.child("First"));
//	m_SecondPoint.LoadFromXML(Node.child("Second"));
	m_SectionLen = Node.attribute("Length").as_float(0.0f);
	return true;
}

cPathNode::cPathNode():m_MasterPoint(), m_FirstPoint(), m_SecondPoint(), m_SectionLen() {
}

cPathNode::cPathNode(const cPathNode &other):
			m_MasterPoint(other.m_MasterPoint), m_FirstPoint(other.m_FirstPoint), m_SecondPoint(other.m_SecondPoint),
			m_SectionLen(other.m_SectionLen){
}

cPathNode::~cPathNode() {
}

#endif

} //namespace Paths 
} //namespace DataClasses 

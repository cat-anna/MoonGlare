/*
	Generated by cppsrc.sh
	On 2015-02-04 22:26:26,44
	by Paweu
*/

#pragma once
#ifndef BezierPath_H
#define BezierPath_H

namespace DataClasses {
namespace Paths {

#if 0
class cPathNode {
protected:
	math::vec3 m_MasterPoint;
	math::vec3 m_FirstPoint, m_SecondPoint;
	float m_SectionLen;
public:
	DefineREADAcces(MasterPoint, math::vec3);
	DefineREADAcces(FirstPoint , math::vec3);
	DefineREADAcces(SecondPoint, math::vec3);
	DefineFULLAcces(SectionLen, float);
	bool SaveToXML(xml_node Node) const;
	bool LoadFromXML(xml_node Node);
	cPathNode();
	cPathNode(const cPathNode &other);
	~cPathNode();
};
class impl {
public:
	typedef std::vector<cPathNode> cPathNodeVector;
	enum ePathFlags {
		pfCircled = 0x0001,
	};
	unsigned GetNodeCount() const { return m_NodeVector.size(); }
	const cPathNode& GetNode(unsigned index) const { return m_NodeVector[index]; }
protected:
	cPathNodeVector m_NodeVector;
	unsigned m_Flags;
};

#endif

} //namespace Paths 
} //namespace DataClasses 

#endif

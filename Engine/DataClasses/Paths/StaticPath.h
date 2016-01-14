/*
	Generated by cppsrc.sh
	On 2015-02-04 22:27:32,09
	by Paweu
*/

#pragma once
#ifndef StaticPath_H
#define StaticPath_H

namespace DataClasses {
namespace Paths {

class StaticPath : public iPath {
	GABI_DECLARE_CLASS(StaticPath, iPath);
public:
 	StaticPath(DataModule *DataModule = 0);
 	virtual ~StaticPath();

	virtual float Length() const override;
	virtual Physics::vec3 PositionAt(float t) const override;
	virtual bool UpdatePosition(PathCalcState &state) const override;

	virtual bool LoadMeta(const xml_node Node) override;

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
private: 
	float m_Length = 0.0f;
	std::vector<Physics::vec3> m_Nodes;
	std::vector<float> m_SectionLengths;
};

} //namespace Paths 
} //namespace DataClasses 

#endif

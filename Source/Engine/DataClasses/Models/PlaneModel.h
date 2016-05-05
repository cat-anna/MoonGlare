/*
	Generated by cppsrc.sh
	On 2015-01-11 18:15:28,46
	by Paweu
*/

#pragma once
#ifndef PlaneModel_H
#define PlaneModel_H

namespace DataClasses {
namespace Models {

class PlaneModel : public iSimpleModel {
	SPACERTTI_DECLARE_STATIC_CLASS(PlaneModel, iSimpleModel);
public:
	PlaneModel(const string &Name);
 	virtual ~PlaneModel();

	virtual bool LoadFromXML(const xml_node Node) override;
protected:
	bool LoadPlanes(xml_node Node);
};

} //namespace Models 
} //namespace DataClasses 

#endif

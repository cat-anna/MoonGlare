/*
 * cModel.h
 *
 *  Created on: 12-11-2013
 *      Author: Paweu
 */

#ifndef CMODEL_H_
#define CMODEL_H_

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class iModel : public DataClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iModel, DataClass);
public:
	iModel(const string& Name);
	virtual ~iModel();

	virtual void DoRender(cRenderDevice &Dev) const = 0;
	virtual void DoRenderMesh(cRenderDevice &Dev) const = 0;
	
//	virtual Physics::SharedShape ConstructShape(float ShapeScale) const;
//	virtual const Physics::PhysicalProperties* GetPhysicalProperties() const;
	
	virtual bool LoadFromXML(const xml_node Node) = 0;
protected:
//	Physics::PhysicalProperties m_PhysicalProperties;

	virtual bool DoInitialize() override;
	virtual bool DoFinalize() override;
};

} // namespace Models 
} // namespace DataClasses 
} //namespace MoonGlare 

#endif // CMODEL_H_ 

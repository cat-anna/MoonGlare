/*
 * ModelMaterial.h
 *
 *  Created on: 17-11-2013
 *      Author: Paweu
 */

#ifndef ModelMaterial_H_
#define ModelMaterial_H_

#include "RootClass.h"

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class ModelMaterial: public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(ModelMaterial, cRootClass);
protected:
	iModel *m_Owner;
	Graphic::Material m_Material;
public:
	ModelMaterial(const ModelMaterial&) = delete;
	ModelMaterial(const ModelMaterial&&) = delete;
	ModelMaterial(iModel *Owner, const aiMaterial *Material);
	ModelMaterial(iModel *Owner, const xml_node MaterialDef, FileSystem::DirectoryReader reader);
	virtual ~ModelMaterial();

	const Graphic::Material& GetMaterial() const { return m_Material; }
	Graphic::Material& GetMaterial() { return m_Material; }
};

typedef boost::ptr_vector<ModelMaterial> MaterialVector;

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 

#endif // ModelMaterial_H_ 

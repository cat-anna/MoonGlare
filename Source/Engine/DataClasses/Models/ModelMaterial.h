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

class ModelMaterial {
	iModel *m_Owner;
	Graphic::Material m_Material;
	Renderer::MaterialResourceHandle m_Handle{ 0 };
public:
	ModelMaterial(const ModelMaterial&) = delete;
	ModelMaterial(const ModelMaterial&&) = delete;
	ModelMaterial(iModel *Owner, const aiMaterial *Material, const aiScene *Scene, Renderer::Resources::ResourceManager *resmgr = nullptr);
	ModelMaterial(iModel *Owner, const xml_node MaterialDef, FileSystem::DirectoryReader reader);
	ModelMaterial(iModel *Owner, const std::string &Edges, const std::string &TexURI, FileSystem::DirectoryReader reader);
	ModelMaterial(iModel *Owner, const std::string &Edges, const std::string &TexURI);

	const Graphic::Material& GetMaterial() const { return m_Material; }
	Graphic::Material& GetMaterial() { return m_Material; }

	Renderer::MaterialResourceHandle GetMaterialHandle() const {
		return m_Handle;
	}
};

typedef std::vector<std::unique_ptr<ModelMaterial>> MaterialVector;

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 

#endif // ModelMaterial_H_ 

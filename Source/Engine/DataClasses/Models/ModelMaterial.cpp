/*
 * ModelMaterial.cpp
 *
 *  Created on: 17-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace DataClasses {
namespace Models {

#define xmlMaterialNode_Texture		"Texture"

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(ModelMaterial)

ModelMaterial::ModelMaterial(iModel *Owner, const aiMaterial *Material) :
				m_Owner(Owner), m_Material() {
	aiString Path;
	if (Material->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
		AddLog(Error, "Unable to load material for model: " << m_Owner->Info());
	}
	FileSystem::DirectoryReader reader(DataPath::Models, m_Owner->GetName());
	reader.OpenTexture(m_Material.Texture, Path.data);
	m_Material.Texture.SetRepeatEdges();
}

ModelMaterial::ModelMaterial(iModel* Owner, const xml_node MaterialDef, FileSystem::DirectoryReader reader) :
			m_Owner(Owner), m_Material()  {

	if (!MaterialDef) {
		AddLog(Error, "Cannot load material from empty node! ");
		return;
	}
	xml_node TextureNode = MaterialDef.child(xmlMaterialNode_Texture);

	string fileName = TextureNode.child("File").text().as_string("??");
	string EdgeConf = TextureNode.attribute("Edges").as_string("?");

	reader.OpenTexture(m_Material.Texture, fileName) ;
	if (! strcmp(EdgeConf.c_str(), "Repeat"))
		Graphic::GetRenderDevice()->RequestContextManip([this, fileName, EdgeConf, reader]{
			m_Material.Texture.SetRepeatEdges();	
		});

	m_Material.AlphaThreshold = TextureNode.attribute("AlphaThreshold").as_float(m_Material.AlphaThreshold);
	
	XML::Vector::Read(MaterialDef, "BackColor", m_Material.BackColor, m_Material.BackColor, XML::Captions::RGBA);
}

ModelMaterial::~ModelMaterial() {
}

} // namespace Models
} // namespace DataClasses
} //namespace MoonGlare 

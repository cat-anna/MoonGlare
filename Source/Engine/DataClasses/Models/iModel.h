/*
 * cModel.h
 *
 *  Created on: 12-11-2013
 *      Author: Paweu
 */

#pragma once

namespace MoonGlare {
namespace DataClasses {
namespace Models {

class iModel : public DataClass {
    SPACERTTI_DECLARE_CLASS(iModel, DataClass);
public:
    iModel(const string& Name = "");
    virtual ~iModel();

    struct MeshData : public Graphic::VAO::MeshData {
        Renderer::MaterialResourceHandle m_Material{ 0 };
    };
    typedef std::vector<MeshData> MeshDataVector;

    Graphic::VAO& GetVAO() { return m_VAO; }
    MeshDataVector& GetMeshVector() { return m_Meshes; }

    virtual bool LoadFromXML(const xml_node Node) {
        return true;
    }

    Renderer::VAOResourceHandle vaoHandle;
protected:
    MeshDataVector m_Meshes;
    Graphic::VAO m_VAO;

    virtual bool DoFinalize() override;
};

} // namespace Models 
} // namespace DataClasses 
} // namespace MoonGlare 


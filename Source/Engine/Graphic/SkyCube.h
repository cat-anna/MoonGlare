/*
    Generated by cppsrc.sh
    On 2015-01-21  6:15:50,14
    by Paweu
*/

#pragma once
#ifndef SkyCube_H
#define SkyCube_H

namespace Graphic {

class SkyCube {
public:
    SkyCube();
    ~SkyCube();

    struct Faces {
        enum {
            Left, Right, Front, Back, Top, Bottom, MaxValue,
        };
    };

    void Render(cRenderDevice &dev) const;

    bool LoadMeta(const xml_node node, FileSystem::DirectoryReader &reader);

    bool Initialize();
    bool Finalize();
protected:
//	Texture m_Textures[6];
    float m_Size = 100;
    math::vec3 m_BackColor = math::vec3(1, 1, 1);
    VAO m_VAO;
    VAO::MeshVector m_Mesh;

    void ResetVAO();
};

} //namespace Graphic 

#endif

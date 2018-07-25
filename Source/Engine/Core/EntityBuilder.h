/*
  * Generated by cppsrc.sh
  * On 2016-08-14 22:04:32,35
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EntityBuilder_H
#define EntityBuilder_H

namespace MoonGlare {
namespace Core {

class EntityBuilder {
public:
    EntityBuilder(Component::SubsystemManager *Manager);
    ~EntityBuilder();

    static bool Build(Component::SubsystemManager *Manager, Entity Owner, const char *PatternUri, Entity &eout, std::string Name) {
        EntityBuilder eb(Manager);
        return eb.Build(Owner, PatternUri, eout, std::move(Name));
    }

    bool Build(Entity Owner, const char *PatternUri, Entity &eout, std::string Name = std::string());
    bool Build(Entity Owner, pugi::xml_node node, std::string Name = std::string());

protected:
    bool LoadComponent(Entity Owner, pugi::xml_node node, Handle &hout);
    Component::SubsystemManager *m_Manager;

    struct ImportData;

    void Import(ImportData &data, const char *PatternUri, int32_t entityIndex);
    void Import(ImportData &data, pugi::xml_node node, int32_t entityIndex);

    void Spawn(ImportData &data, Entity Owner);
};

} //namespace Core 
} //namespace MoonGlare 

#endif

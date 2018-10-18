#include <pch.h>
#include <nfMoonGlare.h>
#include <Foundation/Component/iSubsystem.h>
#include "SubsystemManager.h"
#include "ComponentRegister.h"                        
#include <Foundation/SoundSystem/Component/SoundSourceSystem.h>

namespace MoonGlare::Core::Component {

ComponentRegister::MapType *ComponentRegister::s_ComponentMap = nullptr;

void ComponentRegister::Dump(std::ostream &out) {
    out << "Component Register:\n";
    if (!s_ComponentMap) {
        out << "\tEmpty\n";
        return;
    }

    for (auto &it : *s_ComponentMap) {
        char buffer[128];
        sprintf_s(buffer, "\t%20s 0x%02x[%3d]\n", it.first.c_str(), (unsigned)it.second->m_CID, (unsigned)it.second->m_CID);
        out << buffer;
    }
}

bool ComponentRegister::ExtractCIDFromXML(pugi::xml_node node, SubSystemId & out) {
    auto idxml = node.attribute("Id");
    if (idxml) {
        out = static_cast<SubSystemId>(idxml.as_uint(0));
        return out != (SubSystemId)SubSystemId::Invalid;
    } else {
        auto namexml = node.attribute("Name");
        if (!namexml) {
            AddLogf(Error, "Component definition without id or name!");
            return false;
        }
        if (GetComponentID(namexml.as_string(""), out)) {
            return out != (SubSystemId)SubSystemId::Invalid;
        } else {
            auto r = Component::BaseComponentInfo::GetClassByName(namexml.as_string(""));
            if (r.has_value()) {
                out = static_cast<SubSystemId>(r.value());
                return true;
            }

            AddLogf(Error, "Unknown component name: %s", namexml.as_string(""));
            return false;
        }
    }
    return false;
}

}

#include PCH_HEADER

#include "EditableEntity.h"

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

using Core::Component::ComponentID;
using Core::Component::ComponentID;

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

EditableEntity::EditableEntity(EditableEntity *Parent)
    : m_Parent(Parent) {}

EditableEntity::~EditableEntity() {}

bool EditableEntity::Read(pugi::xml_node node, const char *NodeName) {
    return Read(node.child(NodeName ? NodeName : "Entity"));
}

bool EditableEntity::Read(pugi::xml_node node) {
    m_Name = node.attribute("Name").as_string("");
    m_PatternURI = node.attribute("Pattern").as_string("");

    for (auto it = node.first_child(); it; it = it.next_sibling()) {
        const char *nodename = it.name();
        auto hash = Space::Utils::MakeHash32(nodename);

        switch (hash) {

        case "Component"_Hash32:
        {
            auto child = EditableComponent::CreateComponent(this, it);
            if (!child) {
                //TODO: log sth
                continue;
            }
            child->enabled = it.attribute("Enabled").as_bool(true);
            m_Components.emplace_back(std::move(child));
            break;
        }

        //case "Entity"_Hash32:
        //{
        //auto pattern = it.attribute("Pattern").as_string(nullptr);
        //if (pattern) {
        //	XMLFile xdoc;
        //	Entity child;
        //	std::string paturi = pattern;
        //	if (!GetFileSystem()->OpenXML(xdoc, paturi, DataPath::URI)) {
        //		AddLogf(Error, "Failed to open pattern: %s", pattern);
        //		continue;
        //	}
        //
        //	auto c = BuildChild(Owner, xdoc->document_element(), child);
        //	if (c == 0) {
        //		AddLogf(Error, "Failed to load child!");
        //		continue;
        //	}
        //	count += c;
        //	continue;
        //}
        //}
        //no break;
        //[[fallthrough]]
        case "Entity"_Hash32:
        case "Child"_Hash32:
        {
            UniqueEditableEntity child(new EditableEntity(this));
            if (!child->Read(it)) {
                //TODO: log sth
                continue;
            }
            child->enabled = it.attribute("Enabled").as_bool(true);
            m_Children.emplace_back(std::move(child));
        }
        continue;
        default:
            AddLogf(Warning, "Unknown node: %s", nodename);
            continue;
        }
    }

    return true;
}

bool EditableEntity::Write(pugi::xml_node node, const char *NodeName) {
    return Write(node.append_child(NodeName ? NodeName : "Entity"));
}

bool EditableEntity::Write(pugi::xml_node node) {
    if (!m_Name.empty())
        node.append_attribute("Name") = m_Name.c_str();
    if (!m_PatternURI.empty())
        node.append_attribute("Pattern") = m_PatternURI.c_str();

    bool ret = true;
    for (auto &it : m_Children) {
        auto cnode = node.append_child("Entity");
        ret = ret && it->Write(cnode);
        cnode.append_attribute("Enabled") = it->enabled;
    }
    for (auto &it : m_Components) {
        auto cnode = node.append_child("Component");
        cnode.append_attribute("Name") = it->GetName().c_str();
        cnode.append_attribute("Enabled") = it->enabled;

        ret = ret && it->Write(cnode);
    }
    return ret;
}

void EditableEntity::MoveDown(EditableComponent *c) {
    auto idx = Find(c);
    if (idx < 0)
        return;
    if (idx + 1 >= (int)m_Components.size())
        return;
    m_Components[idx].swap(m_Components[idx + 1]);
}

void EditableEntity::MoveUp(EditableComponent *c) {
    auto idx = Find(c);
    if (idx <= 0)
        return;
    m_Components[idx].swap(m_Components[idx - 1]);
}

void EditableEntity::MoveDown(EditableEntity * c) {
    auto idx = Find(c);
    if (idx < 0)
        return;
    if (idx + 1 >= (int)m_Children.size())
        return;
    m_Children[idx].swap(m_Children[idx + 1]);
}

void EditableEntity::MoveUp(EditableEntity * c) {
    auto idx = Find(c);
    if (idx <= 0)
        return;
    m_Children[idx].swap(m_Children[idx - 1]);
}

EditableEntity *EditableEntity::AddChild() {
    UniqueEditableEntity child(new EditableEntity(this));
    child->GetName() = "child";
    auto rawptr = child.get();
    m_Children.emplace_back(std::move(child));
    return rawptr;
}

void EditableEntity::DeleteChild(EditableEntity * c) {
    auto idx = Find(c);
    if (idx < 0)
        return;

    m_Children[idx].reset();
    for (; idx < (int)m_Children.size() - 1; ++idx)
        m_Children[idx].swap(m_Children[idx + 1]);;

    m_Children.pop_back();
}

EditableComponent* EditableEntity::AddComponent(Core::ComponentID cid) {
    auto child = EditableComponent::CreateComponent(this, cid);
    if (!child) {
        //TODO: log sth
        return nullptr;
    }
    auto rawptr = child.get();
    m_Components.emplace_back(std::move(child));
    return rawptr;
}

void EditableEntity::DeleteComponent(EditableComponent * c) {
    auto idx = Find(c);
    if (idx < 0)
        return;

    m_Components[idx].reset();
    for (; idx < (int)m_Components.size() - 1; ++idx)
        m_Components[idx].swap(m_Components[idx + 1]);;
    m_Components.pop_back();
}

void EditableEntity::Clear() {
    m_Children.clear();
    m_Components.clear();
    m_Name = "Entity";
    m_PatternURI.clear();
}

bool EditableEntity::Serialize(std::string &out) {
    pugi::xml_document xdoc;
    if (!Write(xdoc, "Entity"))
        return false;
    std::stringstream ss;
    xdoc.save(ss);
    out = ss.str();
    return true;
}

bool EditableEntity::Deserialize(std::string &out) {
    pugi::xml_document xdoc;
    if (!xdoc.load_string(out.c_str())) {
        return false;
    }
    return Read(xdoc.document_element());
}

bool EditableEntity::DeserializeToChild(std::string &out) {
    auto ch = AddChild();
    if (ch->Deserialize(out))
        return true;
    DeleteChild(ch);
    return false;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, pugi::xml_node node) {
    ComponentID cid = ComponentID::Invalid;

    auto idxml = node.attribute("Id");
    if (idxml) {
        cid = static_cast<ComponentID>(idxml.as_uint(0));
    }
    else {
        auto namexml = node.attribute("Name");
        if (!namexml) {
            AddLogf(Error, "Component definition without id or name!");
            return nullptr;
        }
        auto ci = TypeEditor::ComponentInfo::GetComponentInfo(namexml.as_string(""));
        if (ci)
            cid = ci->m_CID;
        if (cid == (ComponentID)ComponentID::Invalid) {
            AddLogf(Error, "Unknown component name: %s", namexml.as_string(""));
        }
    }

    if (cid == (ComponentID)ComponentID::Invalid)
        return nullptr;

    auto ret = CreateComponent(Parent, cid);
    if (!ret) {
        //already logged
        return nullptr;
    }

    if (!ret->Read(node)) {
        //TODO: log sth
        return nullptr;
    }

    return std::move(ret);
}

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, MoonGlare::Core::ComponentID cid) {
    auto ci = TypeEditor::ComponentInfo::GetComponentInfo(cid);
    if (!ci) {
        AddLogf(Error, "Unknown component id: %d", (int)cid);
        return nullptr;
    }

    auto x2cs = ci->m_EntryStructure->m_CreateFunc(nullptr);
    return std::make_unique<EditableComponent>(Parent, ci, std::move(x2cs));
}

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare

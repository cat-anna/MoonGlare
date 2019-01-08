#pragma once

#include <ToolBase/UserQuestions.h>
#include <TypeEditor/CustomType.h>
#include <TypeEditor/Structure.h>
#include <ChangesManager.h>
#include <ToolBase/iSettingsUser.h>
#include <ToolBase/Module.h>

#include <Source/Engine/Core/Component/nfComponent.h>

#include <TypeEditor/x2cDataTree.h>
#include <TypeEditor/Structure.h>
#include <TypeEditor/CustomEditorItemDelegate.h>
#include <TypeEditor/ComponentInfo.h>

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

class EditableEntity;
using UniqueEditableEntity = std::unique_ptr<EditableEntity>;
class EditableComponent;
using UniqueEditableComponent = std::unique_ptr<EditableComponent>;

using EditableComponentList = std::vector<UniqueEditableComponent>;
using EditableEntityList = std::vector<UniqueEditableEntity>;

class EditableEntity {
public:
    EditableEntity(EditableEntity *Parent = nullptr);
    virtual ~EditableEntity();

    virtual bool Read(pugi::xml_node node);
    virtual bool Read(pugi::xml_node node, const char *NodeName);
    virtual bool Write(pugi::xml_node node);
    virtual bool Write(pugi::xml_node node, const char *NodeName);

    bool Serialize(std::string &out);
    bool Deserialize(std::string &out);
    bool DeserializeToChild(std::string &out);

    void SetName(std::string Name) { m_Name = Name; }

    std::string& GetName() { return m_Name; }
    const std::string& GetPatternURI() { return m_PatternURI; }
    void SetPatternURI(std::string v) { m_PatternURI.swap(v); }
    EditableEntityList& GetChildrenList() { return m_Children; }
    EditableComponentList& GetComponentList() { return m_Components; }

    EditableEntity* GetParent() const { return m_Parent; }

    void MoveUp(EditableComponent *c);
    void MoveDown(EditableComponent *c);
    void MoveUp(EditableEntity *c);
    void MoveDown(EditableEntity *c);

    int Find(EditableComponent *c) {
        for (int i = 0; i < (int)m_Components.size(); ++i)
            if (m_Components[i].get() == c)
                return i;
        return -1;
    }
    int Find(EditableEntity *c) {
        for (int i = 0; i < (int)m_Children.size(); ++i)
            if (m_Children[i].get() == c)
                return i;
        return -1;
    }

    EditableEntity* AddChild();
    void DeleteChild(EditableEntity *c);
    EditableComponent* AddComponent(const std::string &name);
    void DeleteComponent(EditableComponent *c);
    void Delete() { GetParent()->DeleteChild(this); }

    bool IsDeletable() const { return GetParent() != nullptr; }
    bool IsMovable() const { return GetParent() != nullptr; }

    void Clear();

    bool enabled = true;
protected:
    EditableEntity * m_Parent;
    std::string m_Name;
    std::string m_PatternURI;
    EditableEntityList m_Children;
    EditableComponentList m_Components;
};

class EditableComponent {
public:
    EditableComponent::EditableComponent(EditableEntity *Parent, TypeEditor::SharedComponentInfo cInfo, TypeEditor::UniqueStructure x2cs)
        : m_ComponentInfo(cInfo) {
        m_Parent = Parent;
        m_Data.swap(x2cs);
        m_Data->SetName(m_ComponentInfo->m_Name);
    }
    ~EditableComponent() {}

    bool enabled = true;
    bool active = true;

    const TypeEditor::StructureValueList& GetValues() { return m_Data->GetValues(); }

    bool Read(pugi::xml_node node) { return m_Data->Read(node); }
    bool Write(pugi::xml_node node) { return m_Data->Write(node); }
    const std::string& GetName() { return m_Data->GetName(); }

    struct ValuesIO {
        ValuesIO(const TypeEditor::StructureValueList& values) : values(values) {
            for (auto &it : values) {
                items[it->GetName()] = &it;
            }
        }

        void Set(const std::string &name, const std::string &value) {
            (*items[name])->SetValue(value);
        }
        std::string get(const std::string &name) {
            return (*items[name])->GetValue();
        }
    private:
        std::unordered_map<std::string, const TypeEditor::UniqueStructureValue*> items;
        const TypeEditor::StructureValueList& values;
    };
    std::unique_ptr<ValuesIO> GetValuesEditor() { return std::make_unique<ValuesIO>(GetValues()); }

    static UniqueEditableComponent CreateComponent(EditableEntity *Parent, pugi::xml_node node);
    static UniqueEditableComponent CreateComponent(EditableEntity *Parent, const std::string &name);
protected:
    EditableEntity * m_Parent;
    TypeEditor::SharedComponentInfo m_ComponentInfo;
    TypeEditor::UniqueStructure m_Data;
};


} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare

Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EditableComponent*);
Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EditableEntity*);



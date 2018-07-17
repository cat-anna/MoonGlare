#pragma once

namespace MoonGlare {
namespace TypeEditor {

#ifdef _X2C_IMPLEMENTATION_

template<typename X2CLASS>
class StructureTemplate : public Structure {
public:
    struct EditableValue : public StructureValue {
        const std::string& GetName() override {
            return m_Name;
        }
        std::string GetValue() override {
            std::string v;
            m_Read(v);
            return std::move(v);
        }
        void SetValue(const std::string& v) override {
            m_Write(v);
        }
        const std::string& GetDescription() override {
            return m_Description;
        }
        const std::string& GetTypeName() override {
            return m_Type;
        }
        std::string GetFullName() override {
            return m_Type + ":" + owner->GetName() + "." + m_Name;
        }

        StructureTemplate *owner;
        std::function<void(std::string &output)> m_Read;
        std::function<void(const std::string &input)> m_Write;
        std::string m_Name;
        std::string m_Description;
        std::string m_Type;
    };

    StructureTemplate(Structure *Parent = nullptr) : Structure(Parent) {
        m_TypeName = X2CLASS::GetTypeName();
        m_LocalValue = std::make_unique<X2CLASS>();
        m_Value = m_LocalValue.get();
        m_Value->ResetToDefault();

        ::x2c::cxxpugi::StructureMemberInfoTable members;
        std::unordered_map<std::string, std::function<void(X2CLASS &self, const std::string &input)>> WriteFuncs;
        std::unordered_map<std::string, std::function<void(const X2CLASS &self, std::string &output)>> ReadFuncs;

        m_Value->GetMemberInfo(members);
        m_Value->GetWriteFuncs(WriteFuncs);
        m_Value->GetReadFuncs(ReadFuncs);
        m_Values.reserve(members.size());
        for (auto &member : members) {
            auto ptr = std::make_unique<EditableValue>();
            ptr->m_Name = member.m_Name;
            ptr->m_Description = member.m_Description;
            ptr->m_Type = member.m_TypeName;
            ptr->owner = this;

            auto read = ReadFuncs[member.m_Name];
            auto write = WriteFuncs[member.m_Name];

            ptr->m_Read = [this, read](std::string &out) {
                read(*this->m_Value, out);
            };
            ptr->m_Write = [this, write](const std::string &out) {
                write(*this->m_Value, out);
            };
            m_Values.emplace_back(UniqueStructureValue(ptr.release()));
        }
    }
    ~StructureTemplate() {}

    virtual const StructureValueList& GetValues() override {
        return m_Values;
    }
    virtual bool Read(pugi::xml_node node) override {
        return m_Value->Read(node);
    }
    virtual bool Write(pugi::xml_node node) override {
        return m_Value->Write(node);
    }
    virtual const std::string& GetName() override {
        return m_TypeName;
    }
    virtual void SetName(const std::string& value) override {
        m_TypeName = value;
    }
    virtual void SetExternlDataSink(void *src) override {
        if (!src)
            m_Value = m_LocalValue.get();
        else
            m_Value = (X2CLASS*)src;
    }
    virtual void ResetToDefault() override {
        m_Value->ResetToDefault();
    }
private:
    StructureValueList m_Values;
    X2CLASS *m_Value;
    std::unique_ptr<X2CLASS> m_LocalValue;
    std::string m_TypeName;
};

#endif

} //namespace TypeEditor 
} //namespace MoonGlare 

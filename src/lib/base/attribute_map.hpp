
#pragma once

#include "variant_argument_map.hpp"
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace MoonGlare {

class iAttributeBase {
public:
    virtual ~iAttributeBase() = default;

    virtual std::string GetName() const = 0;
    virtual std::string GetTypeName() const = 0;
};

template <typename OwnerObject> class iAttribute : public iAttributeBase {
public:
    ~iAttribute() override = default;

    using VariantType = VariantArgumentMap::VariantType;

    virtual VariantType GetValue(const OwnerObject *object) const = 0;
    virtual void SetValue(OwnerObject *object, const VariantType &value) const = 0;
};

class iAttributeProviderBase {
public:
    virtual ~iAttributeProviderBase() = default;
    virtual std::vector<iAttributeBase *> GetAttributeInfo() const = 0;
};

template <typename OwnerObject> class iAttributeProvider : public iAttributeProviderBase {
public:
    ~iAttributeProvider() override = default;

    virtual std::vector<iAttribute<OwnerObject> *> GetAttributes() const = 0;

    //?
    // virtual VariantArgumentMap GetAttributeValues() const = 0;
    // virtual void SetAttributeValues(const VariantArgumentMap &argument_map) const = 0;
};

} // namespace MoonGlare

#if 0

namespace StarVFS {

template <class STRING> class BaseAttributeMap {
public:
    BaseAttributeMap() {}
    virtual ~BaseAttributeMap() {}

    using String_t = STRING;
    using SetFunc = void (BaseAttributeMap::*)(const String_t &);
    using GetFunc = const String_t &(BaseAttributeMap::*)() const;

    struct AttributeMapInstance {
        bool Set(BaseAttributeMap *Object, const String_t &Name, const String_t &Value) {
            if (!Object || !ValidateType(Object))
                return false;
            auto it = m_Attribs.find(Name);
            if (it == m_Attribs.end())
                return false;
            auto set = it->second.set;
            if (!set)
                return false;
            (Object->*set)(Value);
            return true;
        }
        bool Get(BaseAttributeMap *Object, const String_t &Name, String_t &Value) const {
            if (!Object || !ValidateType(Object))
                return false;
            auto it = m_Attribs.find(Name);
            if (it == m_Attribs.end())
                return false;
            auto get = it->second.get;
            if (!get)
                return false;
            Value = (Object->*get)();
            return true;
        }
        bool Exists(const String_t &Name) const {
            auto it = m_Attribs.find(Name);
            if (it == m_Attribs.end())
                return false;
            return true;
        }
        std::vector<String> GetAttributeNames() const {
            std::vector<String> v;
            v.reserve(m_Attribs.size());
            for (auto &it : m_Attribs)
                v.emplace_back(it.first);
            return v;
        }

    protected:
        AttributeMapInstance() {}
        AttributeMapInstance(const AttributeMapInstance &) = delete;
        virtual bool ValidateType(BaseAttributeMap *Object) const = 0;

        struct AttribData {
            GetFunc get;
            SetFunc set;
        };

        std::unordered_map<String_t, AttribData> m_Attribs;
    };

    const AttributeMapInstance &GetAttributeMap() const {
        if (!s_AttributeMapInstance)
            s_AttributeMapInstance = GetAttributeMapInstance();
        return *s_AttributeMapInstance;
    }
    AttributeMapInstance &GetAttributeMap() {
        if (!s_AttributeMapInstance)
            s_AttributeMapInstance = GetAttributeMapInstance();
        return *s_AttributeMapInstance;
    }

    bool GetAttribute(const String_t &Name, String_t &Value) const { return GetAttributeMap().Get(this, Name, Value); }
    bool SetAttribute(const String_t &Name, const String_t &Value) { return GetAttributeMap().Set(this, Name, Value); }
    bool AttributeExists(const String_t &Name) const { return GetAttributeMap().Exists(Name); }
    std::vector<String> GetAttributeNames() const { return GetAttributeMap().GetAttributeNames(); }

protected:
    template <class T> struct OwnedAttributeMapInstance final : public AttributeMapInstance {
        void AddAttrib(const String_t &Name, const String_t &(T::*get)() const,
                       void (T::*set)(const String_t &) = nullptr) {
            typename AttributeMapInstance::AttribData ad{
                static_cast<GetFunc>(get),
                static_cast<SetFunc>(set),
            };
            this->m_Attribs.emplace(Name, ad);
        }

    protected:
        virtual bool ValidateType(BaseAttributeMap *Object) const override final {
            return dynamic_cast<T *>(Object) != nullptr;
        }
    };

    template <class T> static std::unique_ptr<OwnedAttributeMapInstance<T>> CreateAttributeMapInstance() {
        return std::make_unique<OwnedAttributeMapInstance<T>>();
    }

    virtual std::unique_ptr<AttributeMapInstance> GetAttributeMapInstance() const { return nullptr; }

private:
    static std::unique_ptr<AttributeMapInstance> s_AttributeMapInstance;
};

template <class STRING>
std::unique_ptr<typename BaseAttributeMap<STRING>::AttributeMapInstance>
    BaseAttributeMap<STRING>::s_AttributeMapInstance;

} // namespace StarVFS

#endif

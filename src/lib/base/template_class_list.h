#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

namespace MoonGlare::Tools {

namespace RTTI {
struct TypeInfo;
}

template <class T> struct StaticClassInfoImpl {
    static const RTTI::TypeInfo *GetStaticTypeInfo() { return nullptr; }
};

template <class T> struct StaticClassInfo : public StaticClassInfoImpl<T> {};

class TemplateClassListBase {
public:
    struct ClassInfoBase {
        const RTTI::TypeInfo *TypeInfo;
        std::string Alias;

        template <class Class>
        ClassInfoBase(Class *dummy, std::string Alias)
            : TypeInfo(StaticClassInfo<Class>::GetStaticTypeInfo()), Alias(std::move(Alias)) {}
    };
};

//----------------------------------------------------------------

template <class KIND = void> struct ClassListRegisterBase {
    static void EnumerateRegisters(std::function<void(const ClassListRegisterBase<KIND> *)> e) {
        for (auto *it : _list)
            e(it);
    }

    virtual const char *GetName() const = 0;
    virtual const char *GetCompilerName() const = 0;
    virtual void EnumerareClasses(std::function<void(const TemplateClassListBase::ClassInfoBase &info)>) const = 0;

protected:
    void Add() { /*_list.push_back(this);*/
    }
    void Del() { /*_list.remove(this); */
    }

private:
    static std::list<ClassListRegisterBase<KIND> *> _list;
};

template <class KIND> std::list<ClassListRegisterBase<KIND> *> ClassListRegisterBase<KIND>::_list;

template <class REGISTER, class KIND = void> struct ClassRegisterRegister : public ClassListRegisterBase<KIND> {
    ClassRegisterRegister(REGISTER *r) : m_Register(r) { Add(); }
    ~ClassRegisterRegister() { Del(); }

    virtual const char *GetName() const { return "[?]"; /*INFO::GetName();*/ }
    virtual const char *GetCompilerName() const { return typeid(REGISTER).name(); }
    virtual void EnumerareClasses(std::function<void(const TemplateClassListBase::ClassInfoBase &info)> f) const {
        m_Register->Enumerate(f);
    }

private:
    REGISTER *m_Register;
};

//----------------------------------------------------------------

template <class Base, class... Args> class TemplateClassList : public TemplateClassListBase {
public:
    using ThisClass = TemplateClassList<Base, Args...>;

    using Shared = std::shared_ptr<Base>;
    using Unique = std::unique_ptr<Base>;

    using ConstructorFun = Base *(*)(Args...);
    using SharedConstructorFun = Shared (*)(Args...);
    using UniqueConstructorFun = Unique (*)(Args...);

    template <class T> static Base *ConstructorTemplate(Args... args) { return new T(std::forward<Args>(args)...); }

    template <class T> static Unique ConstructorTemplateUnique(Args... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <class T> static Shared ConstructorTemplateShared(Args... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    struct ClassInfo : public ClassInfoBase {
        ConstructorFun Create;
        UniqueConstructorFun UniqueCreate;
        SharedConstructorFun SharedCreate;

        template <class Class>
        ClassInfo(Class *dummy, std::string Alias)
            : ClassInfoBase(dummy, std::move(Alias)), Create(&ConstructorTemplate<Class>),
              UniqueCreate(&ConstructorTemplateUnique<Class>), SharedCreate(&ConstructorTemplateShared<Class>) {}
    };

    typedef std::unordered_map<std::string, ClassInfo> TypeList;

    TemplateClassList() : m_Info(this) {}
    ~TemplateClassList() {}

    template <class T> void Register(const char *Alias = nullptr) {
        if (!Alias) {
            // auto type = StaticClassInfo<T>::GetStaticTypeInfo();
            // if (type)
            // Alias = type->GetName();
            // else
            Alias = typeid(T).name();
        }
        T *dummy = nullptr; // value is not needed, but setting it o nullptr kills dummy warnings
        m_List.insert(std::make_pair(Alias, ClassInfo(dummy, std::move(Alias))));
    }

    Base *CreateClass(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.Create(std::forward<Args>(args)...);
    }

    Unique CreateUnique(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.UniqueCreate(std::forward<Args>(args)...);
    }

    Shared CreateShared(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.SharedCreate(std::forward<Args>(args)...);
    }

    template <class func> void Enumerate(func f) const {
        for (auto &it : m_List)
            f(it.second);
    }

private:
    TypeList m_List;
    ClassRegisterRegister<ThisClass> m_Info;
};

//----------------------------------------------------------------

template <class Base, class Deleter, class... Args> class TemplateClassListDeleter : public TemplateClassListBase {
public:
    using ThisClass = TemplateClassListDeleter<Base, Deleter, Args...>;

    using Shared = std::shared_ptr<Base>;
    using Unique = std::unique_ptr<Base, Deleter>;

    using ConstructorFun = Base *(*)(Args...);
    using SharedConstructorFun = Shared (*)(Args...);
    using UniqueConstructorFun = Unique (*)(Args...);

    template <class T> static Base *ConstructorTemplate(Args... args) { return new T(std::forward<Args>(args)...); }

    template <class T> static Unique ConstructorTemplateUnique(Args... args) {
        return Unique(new T(std::forward<Args>(args)...), Deleter());
    }

    template <class T> static Shared ConstructorTemplateShared(Args... args) {
        return Shared(new T(std::forward<Args>(args)...), Deleter());
    }

    struct ClassInfo : public ClassInfoBase {
        ConstructorFun Create;
        UniqueConstructorFun UniqueCreate;
        SharedConstructorFun SharedCreate;

        template <class Class>
        ClassInfo(Class *dummy, std::string Alias)
            : ClassInfoBase(dummy, std::move(Alias)), Create(&ConstructorTemplate<Class>),
              UniqueCreate(&ConstructorTemplateUnique<Class>), SharedCreate(&ConstructorTemplateShared<Class>) {}
    };

    typedef std::unordered_map<std::string, ClassInfo> TypeList;

    TemplateClassListDeleter() : m_Info(this) {}
    ~TemplateClassListDeleter() {}

    template <class T> void Register(const char *Alias = nullptr) {
        if (!Alias) {
            auto type = StaticClassInfo<T>::GetStaticTypeInfo();
            if (type)
                Alias = type->GetName();
            else
                Alias = typeid(T).name();
        }
        T *dummy = nullptr; // value is not needed, but setting it to nullptr kills dummy warnings
        m_List.insert(std::make_pair(Alias, ClassInfo(dummy, Alias)));
    }

    Base *CreateClass(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.Create(std::forward<Args>(args)...);
    }

    Unique CreateUnique(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.UniqueCreate(std::forward<Args>(args)...);
    }

    Shared CreateShared(const std::string &ClassName, Args... args) const {
        auto it = m_List.find(ClassName);
        if (it == m_List.end())
            return nullptr;
        return it->second.SharedCreate(std::forward<Args>(args)...);
    }

    template <class func> void Enumerate(func f) const {
        for (auto &it : m_List)
            f(it.second);
    }

private:
    TypeList m_List;
    ClassRegisterRegister<ThisClass> m_Info;
};

} // namespace MoonGlare::Tools

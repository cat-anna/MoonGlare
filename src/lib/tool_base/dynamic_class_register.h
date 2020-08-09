#pragma once

#include "template_class_list.h"

namespace MoonGlare::Tools {

template <class Base, class... Args> struct DynamicClassRegister {
    using ClassRegister = TemplateClassList<Base, Args...>;
    using ThisClass = DynamicClassRegister<Base, Args...>;
    using Shared = typename ClassRegister::Shared;
    using Unique = typename ClassRegister::Unique;

    template <class Derived> struct Register {
        Register(const char *Alias = nullptr) { GetRegister()->Register<Derived>(Alias); }
    };

    template <class Derived> void DoRegister(const char *Alias = nullptr) { GetRegister()->Register<Derived>(Alias); }

    static Base *CreateClass(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateClass(ClassName, std::forward<Args>(args)...);
    }

    static Unique CreateUnique(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateUnique(ClassName, std::forward<Args>(args)...);
    }

    static Shared CreateShared(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateShared(ClassName, std::forward<Args>(args)...);
    }

    static ClassRegister *GetRegister() {
        static ClassRegister *_ClassRegister = nullptr;
        if (!_ClassRegister)
            _ClassRegister = new ClassRegister();
        return _ClassRegister;
    }

private:
};

//----------------------------------------------------------------

template <class Base, class Deleter, class... Args> struct DynamicClassRegisterDeleter {
    using ClassRegister = TemplateClassListDeleter<Base, Deleter, Args...>;
    using ThisClass = DynamicClassRegisterDeleter<Base, Deleter, Args...>;
    using Shared = typename ClassRegister::Shared;
    using Unique = typename ClassRegister::Unique;

    template <class Derived> struct Register {
        Register(const char *Alias = nullptr) { GetRegister()->Register<Derived>(Alias); }
    };

    static Base *CreateClass(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateClass(ClassName, std::forward<Args>(args)...);
    }

    static Unique CreateUnique(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateUnique(ClassName, std::forward<Args>(args)...);
    }

    static Shared CreateShared(const std::string &ClassName, Args... args) {
        return GetRegister()->CreateShared(ClassName, std::forward<Args>(args)...);
    }

    static ClassRegister *GetRegister() {
        static ClassRegister *_ClassRegister = nullptr;
        if (!_ClassRegister)
            _ClassRegister = new ClassRegister();
        return _ClassRegister;
    }

private:
};

//----------------------------------------------------------------

} // namespace MoonGlare::Tools


#pragma once

#include <any>
#include <functional>
#include <typeindex>
#include <typeinfo>

namespace MoonGlare {

class iInterfaceHooks {
public:
    virtual ~iInterfaceHooks() = default;

    template <typename Iface>
    void InstallInterfaceHook(std::function<void(Iface *)> functor) {
        InsertInterfaceHook(typeid(Iface), [functor = std::move(functor)](std::any any_ptr) {
            auto ptr = std::any_cast<Iface *>(any_ptr);
            functor(ptr);
        });
    }

    template <typename Iface>
    void InterfaceReady(Iface *iface) {
        RunInterfaceHook(typeid(Iface), std::any(iface));
    }

protected:
    virtual void RunInterfaceHook(const std::type_info &info, std::any any_ptr) = 0;
    virtual void InsertInterfaceHook(const std::type_info &info, std::function<void(std::any)> functor) = 0;
};

} // namespace MoonGlare

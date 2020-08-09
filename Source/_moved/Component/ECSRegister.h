#pragma once

#include "ComponentInfo.h"
#include "EventInfo.h"
#include "SystemInfo.h"

namespace MoonGlare::Component {

struct ECSRegister {
    template<typename T, typename WRAP = nullptr_t> 
    void Component() { 
        ComponentInfo<T>::GetClassId();
        if constexpr(!std::is_same_v<WRAP, nullptr_t>)
            ComponentInfo<T>::SetScriptWrapper<WRAP>();
    }

    template<typename T>
    void System() {
        SystemInfo<T>::GetClassId();
    }

    template<typename T>
    void Event() {
        EventInfo<T>::GetClassId();
    }

    template<typename T>
    void InstallModule() {
        T t;
        t.Register(*this);
    }

    static void Dump();
};

}

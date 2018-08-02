#pragma once

#include <any>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

namespace MoonGlare {

class InterfaceMap {
public:
    template<typename T>
    void SetSharedInterface(std::shared_ptr<T> t) {
        AddInterface(t.get(), t);
    }

    template<typename T>
    void SetInterface(T* t) {
        AddInterface<T>(t, nullptr);
    }

    template<typename T>
    T* GetInterface() {
        auto it = interfaces.find(std::type_index(typeid(T)));
        if (it == interfaces.end()) {
            auto st = GetSharedInterface<T>();
            if (st)
                return st.get();
            AddLogf(Error, "There is no interface %s", typeid(T).name());
            return nullptr;
        }
        try {
            return std::any_cast<T*>(it->second);
        }
        catch (const std::bad_any_cast &e) {
            AddLogf(Error, e.what());
            __debugbreak();
            throw;
        }
    }
    template<typename T>
    std::shared_ptr<T> GetSharedInterface() {
        auto it = sharedInterfaces.find(std::type_index(typeid(T)));
        if (it == sharedInterfaces.end()) {
            AddLogf(Error, "There is no shared interface %s", typeid(T).name());
            return nullptr;
        }
        try {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        catch (const std::bad_any_cast &e) {
            AddLogf(Error, e.what());
            __debugbreak();
            throw;
        }
    }
protected:
    void ReleaseAllInterfaces() {
        interfaces.clear();
        sharedInterfaces.clear();
    }
private:
    std::unordered_map<std::type_index, std::any> interfaces;
    std::unordered_map<std::type_index, std::any> sharedInterfaces;

    template<typename T>
    void AddInterface(T* ptr, std::shared_ptr<T> sptr) {
        auto sPtrIt = sharedInterfaces.find(std::type_index(typeid(T)));
        if (sPtrIt != sharedInterfaces.end()) {
            __debugbreak();
            throw std::runtime_error("Interface already exists!");
        }
        auto ptrIt = interfaces.find(std::type_index(typeid(T)));
        if (ptrIt != interfaces.end()) {
            __debugbreak();
            throw std::runtime_error("Interface already exists!");
        }
        if(sptr)
            sharedInterfaces[std::type_index(typeid(T))] = sptr;
        if (ptr)
            interfaces[std::type_index(typeid(T))] = ptr;
    }
};

}

#pragma once

#include <boost/noncopyable.hpp>

#include <any>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#include "OrbitLoggerConf.h"

namespace MoonGlare {

class InterfaceMap : private boost::noncopyable {
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

    struct NotExistsException :public std::runtime_error {
        NotExistsException(std::string str) :runtime_error(std::move(str)) {}
    };

    template<typename T, typename ... ARGS>
    void CreateObject(ARGS&& ... args) {
        SetSharedInterface(std::make_shared<T>(*this, std::forward<ARGS>(args)...));
    }

    //throws NotExistsException on error
    template<typename T>
    void GetObject(T *& t) {
        t = GetInterface<T>();
        if (!t)
            throw NotExistsException(fmt::format("Object of type {} does not exists", typeid(T).name()));
    }
    //throws NotExistsException on error
    template<typename T>
    void GetObject(std::shared_ptr<T> & t) {
        t = GetSharedInterface<T>();
        if (!t)
            throw NotExistsException(fmt::format("Object of type {} does not exists", typeid(T).name()));
    }

    void DumpObjects() const {
        std::stringstream ss;

        ss << "Shared[" << sharedInterfaces.size() << "]:\n";
        for (auto&[index, any] : sharedInterfaces)
            ss << "\t" << any.type().name() << "\n";
        ss << "Raw[" << interfaces.size() << "]:\n";
        for (auto&[index, any] : interfaces)
            ss << "\t" << any.type().name() << "\n";

        AddLog(Resources, "InterfaceMap " << this << " content:\n" << ss.str());
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

#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace MoonGlare {

namespace detail {

struct EnumConverterBase {
    struct ConvertError : public std::runtime_error {
        ConvertError() : std::runtime_error("Enum conversion error!") {}
    };
};

} // namespace detail

template <class ENUM, ENUM Default = static_cast<ENUM>(0)>
struct EnumConverter : public detail::EnumConverterBase {
    using Enum = ENUM;
    using string = std::string;
    static Enum GetDefaultValue() { return Default; }

    Enum ConvertThrow(const char *cstr) {
        if (cstr == nullptr) {
            throw ConvertError();
        }
        return ConvertThrow(string(cstr));
    }

    Enum ConvertThrow(std::string_view sv) { return ConvertThrow(string(sv)); }

    Enum ConvertThrow(const string &Value) {
        auto it = map.find(Value);
        if (it == map.end()) {
            throw ConvertError();
        }
        return it->second;
    }

    Enum ConvertSafe(const char *cstr) {
        if (cstr == nullptr) {
            return Default;
        }
        return ConvertSafe(string(cstr));
    }

    Enum ConvertSafe(const string &Value) {
        auto it = map.find(Value);
        if (it == map.end()) {
            return Default;
        }
        return it->second;
    }

    const string &ToString(Enum val) {
        for (auto &it : map) {
            if (it.second == val) {
                return it.first;
            }
        }
        if (val == Default) {
            throw ConvertError();
        };
        return ToString(Default);
    }

    bool Convert(const string &Value, Enum &Out) {
        auto it = map.find(Value);
        if (it == map.end()) {
            Out = Default;
            return false;
        } else {
            Out = it->second;
            return true;
        }
    }

protected:
    void Add(const char *name, Enum value) { map[name] = value; }

private:
    using MapType = std::unordered_map<string, Enum>;
    MapType map;
};

template <class ConverterClass>
struct EnumConverterHolder {
    using Enum = typename ConverterClass::Enum;
    using string = typename ConverterClass::string;
    static Enum GetDefaultValue() { return ConverterClass::GetDefault; }

    template <class T>
    static Enum ConvertThrow(T &&Value) {
        return _Instance.ConvertThrow(Value);
    }
    template <class T>
    static Enum ConvertSafe(T &&Value) {
        return _Instance.ConvertSafe(Value);
    }

    static bool Convert(const string &Value, Enum &Out) { return _Instance.Convert(Value, Out); }
    static const string &ToString(Enum val) { return _Instance.ToString(val); }

private:
    static ConverterClass _Instance;
};

template <class ConverterClass>
ConverterClass EnumConverterHolder<ConverterClass>::_Instance;

} // namespace MoonGlare

#include "rect_enums.hpp"

namespace MoonGlare::Component::Rect {

void from_json(const nlohmann::json &j, AlignMode &mode) {
    if (j.is_number_integer()) {
        mode = static_cast<AlignMode>(j.get<int>());
        return;
    }

    if (j.is_string()) {
        mode = AlignModeEnum::ConvertThrow(j.get<std::string>());
        return;
    }

    throw std::runtime_error("Cannot convert json node to AlignMode");
}

void from_json(const nlohmann::json &j, TextAlignMode &mode) {
    if (j.is_number_integer()) {
        mode = static_cast<TextAlignMode>(j.get<int>());
        return;
    }

    if (j.is_string()) {
        mode = TextAlignModeEnum::ConvertThrow(j.get<std::string>());
        return;
    }

    throw std::runtime_error("Cannot convert json node to TextAlignModeEnum");
}

} // namespace MoonGlare::Component::Rect

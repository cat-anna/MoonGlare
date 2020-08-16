#include "widget_settings_provider.hpp"
#include <QHeaderView>
#include <QTreeView>
#include <QWidget>
#include <orbit_logger.h>
#include <qmainwindow.h>

namespace MoonGlare::Tools::RuntineModules {

namespace {

struct WidgetState {
    std::string geometry;
    std::string state;
    std::string columns;
};

void to_json(nlohmann::json &j, const WidgetState &p) {
    j = {
        {"geometry", p.geometry},
        {"state", p.state},
    };
}

void from_json(const nlohmann::json &j, WidgetState &p) {
    j.at("geometry").get_to(p.geometry);
    j.at("state").get_to(p.state);
}

using WidgetStateMap = std::unordered_map<std::string, WidgetState>;

WidgetState SaveWidgetState(const QWidget *widget) {
    WidgetState widget_state;
    if (widget) {
        widget_state.geometry = widget->saveGeometry().toBase64().toStdString();

        auto *main_window = dynamic_cast<const QMainWindow *>(widget);
        if (main_window != nullptr) {
            widget_state.state = main_window->saveState().toBase64().toStdString();
        }

        auto *header_view = dynamic_cast<const QHeaderView *>(widget);
        if (header_view != nullptr) {
            widget_state.state = header_view->saveState().toBase64().toStdString();
        }
    }
    return widget_state;
}

void RestoreWidgetState(WidgetStateMap &state_map, WidgetStateMap::iterator widget_state_it,
                        QWidget *widget) {
    if (widget && widget_state_it != state_map.end()) {
        const auto &widget_state = widget_state_it->second;
        if (!widget_state.geometry.empty()) {
            widget->restoreGeometry(
                QByteArray::fromBase64(QByteArray::fromStdString(widget_state.geometry)));
        }

        auto *main_window = dynamic_cast<QMainWindow *>(widget);
        if (main_window != nullptr && !widget_state.state.empty()) {
            main_window->restoreState(
                QByteArray::fromBase64(QByteArray::fromStdString(widget_state.state)));
        }

        auto *header_view = dynamic_cast<QHeaderView *>(widget);
        if (header_view != nullptr && !widget_state.state.empty()) {
            header_view->restoreState(
                QByteArray::fromBase64(QByteArray::fromStdString(widget_state.state)));
        }

        state_map.erase(widget_state_it);
    }
}

} // namespace

void iWidgetSettingsProvider::DoSaveSettings(nlohmann::json &json) const {
    iSettingsProvider::DoSaveSettings(json);

    try {
        WidgetStateMap states;

        for (auto &item : GetStateSavableWidgets()) {
            states[item.first] = SaveWidgetState(item.second);
        }
        states["this"] = SaveWidgetState(dynamic_cast<const QWidget *>(this));

        json["widget_state"] = states;
    } catch (const std::exception &e) {
        AddLogf(Error, "Failed to save widget settings: %s", e.what());
    }
}

void iWidgetSettingsProvider::DoLoadSettings(const nlohmann::json &json) {
    iSettingsProvider::DoLoadSettings(json);

    try {
        if (json.contains("widget_state")) {
            auto states = json["widget_state"].get<WidgetStateMap>();
            RestoreWidgetState(states, states.find("this"), dynamic_cast<QWidget *>(this));
            for (auto &item : GetStateSavableWidgets()) {
                RestoreWidgetState(states, states.find(item.first), item.second);
            }
        }
    } catch (const std::exception &e) {
        AddLogf(Warning, "Failed to load widget settings: %s", e.what());
    }
}

} // namespace MoonGlare::Tools::RuntineModules

// template <class T> static void SaveColumns(pugi::xml_node node, const char *Name, T *widget, unsigned count)
// {
//     node = XML::UniqeChild(node, Name);
//     for (unsigned it = 0; it < count; ++it) {
//         char buf[16];
//         sprintf_s(buf, "col%02u", it);
//         XML::UniqeAttrib(node, buf) = widget->columnWidth(it);
//     }
// }
// template <class T> static void LoadColumns(pugi::xml_node node, const char *Name, T *widget, unsigned count)
// {
//     node = node.child(Name);
//     if (!node)
//         return;
//     for (unsigned it = 0; it < count; ++it) {
//         char buf[16];
//         sprintf_s(buf, "col%02u", it);
//         auto att = node.attribute(buf);
//         if (!att)
//             continue;
//         widget->setColumnWidth(it, att.as_uint());
//     }
// }

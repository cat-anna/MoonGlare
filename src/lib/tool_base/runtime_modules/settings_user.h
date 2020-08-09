#pragma once

#include <XMLUtils.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class iSettings : public iModule {
public:
    iSettings(SharedModuleManager modmgr);
    virtual ~iSettings() override;

    bool Initialize() override;
    bool Finalize() override;

    void Save();
    void Load();

    static iSettings *GetiSettings();

    struct iSettingsUser {
        virtual ~iSettingsUser() {}

        void SaveSettings();
        void LoadSettings();
        virtual bool DoSaveSettings(pugi::xml_node node) const = 0;
        virtual bool DoLoadSettings(const pugi::xml_node node) = 0;
        const std::string &GetSettingID() const { return m_SettingID; }
        void SetSettingID(std::string v) { m_SettingID.swap(v); }

    protected:
        template <class T> static void SaveGeometry(pugi::xml_node node, T *widget, const char *Name) {
            XML::UniqeChild(node, Name).text() = widget->saveGeometry().toHex().constData();
        }
        template <class T> static void LoadGeometry(const pugi::xml_node node, T *widget, const char *Name) {
            widget->restoreGeometry(QByteArray::fromHex(node.child(Name).text().as_string()));
        }

        template <class T> static void SaveState(pugi::xml_node node, T *widget, const char *Name) {
            XML::UniqeChild(node, Name).text() = widget->saveState().toHex().constData();
        }
        template <class T> static void LoadState(const pugi::xml_node node, T *widget, const char *Name) {
            widget->restoreState(QByteArray::fromHex(node.child(Name).text().as_string()));
        }

        template <class T> static void SaveColumns(pugi::xml_node node, const char *Name, T *widget, unsigned count) {
            node = XML::UniqeChild(node, Name);
            for (unsigned it = 0; it < count; ++it) {
                char buf[16];
                sprintf_s(buf, "col%02u", it);
                XML::UniqeAttrib(node, buf) = widget->columnWidth(it);
            }
        }
        template <class T> static void LoadColumns(pugi::xml_node node, const char *Name, T *widget, unsigned count) {
            node = node.child(Name);
            if (!node)
                return;
            for (unsigned it = 0; it < count; ++it) {
                char buf[16];
                sprintf_s(buf, "col%02u", it);
                auto att = node.attribute(buf);
                if (!att)
                    continue;
                widget->setColumnWidth(it, att.as_uint());
            }
        }
        static bool SaveChildSettings(pugi::xml_node node, iSettingsUser *child) {
            auto &group = child->GetSettingID();
            if (group.empty())
                return false;
            node = XML::UniqeChild(node, group.c_str());
            return child->DoSaveSettings(node);
        }
        static bool LoadChildSettings(pugi::xml_node node, iSettingsUser *child) {
            auto &group = child->GetSettingID();
            if (group.empty())
                return false;
            node = node.child(group.c_str());
            return child->DoLoadSettings(node);
        }

        void EnableSettings(bool v) { m_EnableSettings = v; }

    private:
        std::string m_SettingID;
        bool m_EnableSettings = true;
    };

    struct CustomConfigSet {
        virtual ~CustomConfigSet() = default;
        virtual void SaveSettings(pugi::xml_node node) = 0;
        virtual void LoadSettings(pugi::xml_node node) = 0;
        virtual std::string GetName() const = 0;
    };

protected:
    void SaveSettings(iSettingsUser *user);
    void LoadSettings(iSettingsUser *user);
    pugi::xml_node CustomSettingsRoot();
    pugi::xml_node DynamicSettingsRoot();

    std::unique_ptr<pugi::xml_document> m_SettingsDoc;
    std::string m_FileName;
};

using iSettingsUser = iSettings::iSettingsUser;

} // namespace MoonGlare::Tools::RuntineModules

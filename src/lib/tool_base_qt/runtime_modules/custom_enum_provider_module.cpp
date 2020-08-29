#include "custom_enum_provider_module.hpp"
#include "custom_type_editor.hpp"
#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <variant_argument_map.hpp>

namespace MoonGlare::Tools::RuntineModules {

class CustomEnumEditor : public iCustomTypeEditor, public QComboBox {
public:
    using VariantType = VariantArgumentMap::VariantType;

    CustomEnumEditor(QWidget *Parent, std::shared_ptr<iCustomEnum> custom_enum)
        : QComboBox(Parent), custom_enum(std::move(custom_enum)) {
        for (auto &item : this->custom_enum->GetValues()) {
            addItem(item.caption.c_str(), QVariant::fromValue(item.value));
        }

        setInsertPolicy(QComboBox::NoInsert);
        model()->sort(0);
        setEditable(false);
    }

    static std::string ToDisplayText(const VariantType &in) { return VariantArgumentMap::CastVariant<std::string>(in); }

    void SetValue(VariantType in) override { setCurrentText(QString::fromStdString(ToDisplayText(in))); }
    VariantType GetValue() override {
        //
        return VariantType(qvariant_cast<std::any>(currentData()));
    }

protected:
    void showPopup() override {
        fixComboBoxDropDownListSizeAdjustment();
        QComboBox::showPopup();
    }

    void fixComboBoxDropDownListSizeAdjustment() {
        int scroll = count() <= maxVisibleItems()
                         ? 0
                         : QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_ScrollBarExtent);

        int max = 0;

        for (int i = 0; i < count(); i++) {
            int width = view()->fontMetrics().horizontalAdvance(itemText(i));
            if (max < width)
                max = width;
        }

        view()->setMinimumWidth(scroll + max);
    }

private:
    std::shared_ptr<iCustomEnum> custom_enum;
};

//----------------------------------------------------------------------------------

struct CustomEnumEditorFactory : public iCustomTypeEditorFactory {
    CustomEnumEditorFactory(std::shared_ptr<iCustomEnum> e) : custom_enum(std::move(e)) {}

    iCustomTypeEditor *CreateEditor(QWidget *parent) const override {
        return new CustomEnumEditor(parent, custom_enum);
    };
    std::string ToDisplayText(const VariantArgumentMap::VariantType &in) const override {
        return CustomEnumEditor::ToDisplayText(in);
    }

private:
    std::shared_ptr<iCustomEnum> custom_enum;
};

struct BoolEnumEditorFactory : public CustomEnumEditorFactory {
    BoolEnumEditorFactory(std::shared_ptr<iCustomEnum> e) : CustomEnumEditorFactory(std::move(e)) {}

    std::string ToDisplayText(const VariantArgumentMap::VariantType &in) const override {
        auto v = VariantArgumentMap::CastVariant<bool>(in);
        return v ? "true" : "false";
    }
};

//----------------------------------------------------------------------------------

CustomEnumProviderModule::CustomEnumProviderModule(SharedModuleManager modmgr) : iModule(modmgr) {
}

bool CustomEnumProviderModule::Initialize() {
    custom_type_editor_provider = GetModuleManager()->QueryModule<iCustomTypeEditorProvider>();

    auto bool_enum = std::make_shared<ConstEnum>("bool", std::vector<iCustomEnum::EnumValue>{
                                                             {"false", std::any(false)},
                                                             {"true", std::any(true)},
                                                         });
    RegisterEnum(bool_enum, false);
    custom_type_editor_provider->RegisterTypeEditor(std::make_shared<BoolEnumEditorFactory>(bool_enum),
                                                    bool_enum->GetEnumTypeName());
    return true;
}

bool CustomEnumProviderModule::PostInit() {
    for (auto module : GetModuleManager()->QueryInterfaces<iCustomEnumSupplier>()) {
        for (auto e : module.interface->GetCustomEnums()) {
            RegisterEnum(e.second, e.first);
        }
    }
    return true;
}

void CustomEnumProviderModule::RegisterEnum(std::shared_ptr<iCustomEnum> e, bool wants_type_editor) {
    AddLogf(Info, "Registered enum %s", e->GetEnumTypeName().c_str());
    if (wants_type_editor) {
        custom_type_editor_provider->RegisterTypeEditor(std::make_shared<CustomEnumEditorFactory>(e),
                                                        e->GetEnumTypeName());
    }
    enum_map[e->GetEnumTypeName()] = std::move(e);
}

std::shared_ptr<iCustomEnum> CustomEnumProviderModule::GetEnum(const std::string &type_name) const {
    auto it = enum_map.find(type_name);
    if (it == enum_map.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace MoonGlare::Tools::RuntineModules

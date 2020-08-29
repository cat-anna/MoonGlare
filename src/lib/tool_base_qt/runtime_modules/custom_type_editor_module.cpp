#include "custom_type_editor_module.hpp"
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <orbit_logger.h>

namespace MoonGlare::Tools::RuntineModules {

using VariantType = VariantArgumentMap::VariantType;

//----------------------------------------------------------------------------------

class FloatEditor : public iCustomTypeEditor, public QDoubleSpinBox {
public:
    FloatEditor(QWidget *Parent) : QDoubleSpinBox(Parent) {
        setSingleStep(0.1);
        setMinimum(-1.0e5f);
        setMaximum(1.0e5f);
        setDecimals(5);
    }
    static std::string ToDisplayText(const VariantType &in) {
        auto v = VariantArgumentMap::CastVariant<double>(in);
        return fmt::format("{:.5f}", v);
    }

    virtual void SetValue(VariantType in) {
        auto f = VariantArgumentMap::CastVariant<double>(in);
        QDoubleSpinBox::setValue(f);
    }
    virtual VariantType GetValue() { return VariantType(value()); }
};

//----------------------------------------------------------------------------------

template <class T>
class IntegralEditor : public iCustomTypeEditor, public QSpinBox {
public:
    IntegralEditor(QWidget *Parent) : QSpinBox(Parent) {
        setSingleStep(1);
        setMinimum(std::numeric_limits<T>::min());
        setMaximum(std::numeric_limits<T>::max());
    }
    static std::string ToDisplayText(const VariantType &in) { return VariantArgumentMap::CastVariant<std::string>(in); }

    virtual void SetValue(VariantType in) {
        auto v = VariantArgumentMap::CastVariant<int64_t>(in);
        QSpinBox::setValue(v);
    }
    virtual VariantType GetValue() { return VariantType(static_cast<int64_t>(value())); }
};

//----------------------------------------------------------------------------------

CustomTypeEditorProviderModule::CustomTypeEditorProviderModule(SharedModuleManager modmgr) : iModule(modmgr) {
}

bool CustomTypeEditorProviderModule::PostInit() {
    RegisterEditor<IntegralEditor<int8_t>, int8_t>();
    RegisterEditor<IntegralEditor<uint8_t>, uint8_t>();
    RegisterEditor<IntegralEditor<int16_t>, int16_t>();
    RegisterEditor<IntegralEditor<uint16_t>, uint16_t>();
    RegisterEditor<IntegralEditor<int32_t>, int32_t>();
    RegisterEditor<IntegralEditor<uint32_t>, uint32_t>();
    RegisterEditor<IntegralEditor<int64_t>, int64_t>();
    RegisterEditor<IntegralEditor<uint64_t>, uint64_t>();
    RegisterEditor<FloatEditor, float>();
    RegisterEditor<FloatEditor, double>();
    return true;
}

void CustomTypeEditorProviderModule::RegisterTypeEditor(std::shared_ptr<iCustomTypeEditorFactory> factory,
                                                        const std::string &type_name) {
    AddLogf(Info, "Registered type editor for %s", type_name.c_str());
    type_factories[type_name] = std::move(factory);
}

std::shared_ptr<iCustomTypeEditorFactory>
CustomTypeEditorProviderModule::GetEditorFactory(const std::string &type_name) {
    auto it = type_factories.find(type_name);
    if (it != type_factories.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace MoonGlare::Tools::RuntineModules

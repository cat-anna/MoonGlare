#pragma once

#include <QWidget>
#include <any>
#include <runtime_modules.h>
#include <string>
#include <variant_argument_map.hpp>

namespace MoonGlare::Tools {

class iCustomTypeEditor {
public:
    virtual ~iCustomTypeEditor() = default;

    virtual void SetValue(VariantArgumentMap::VariantType) = 0;
    virtual VariantArgumentMap::VariantType GetValue() = 0;

    virtual QWidget *GetWidget() { return dynamic_cast<QWidget *>(this); }
    virtual void SetDataSource(std::function<QVariant(int)>){};
    void SetModuleManager(SharedModuleManager smm) { sharedModuleManager.swap(smm); }

protected:
    SharedModuleManager GetModuleManager() const {
        assert(sharedModuleManager);
        return sharedModuleManager;
    }

private:
    SharedModuleManager sharedModuleManager;
};

class iCustomTypeEditorFactory {
public:
    virtual ~iCustomTypeEditorFactory() = default;
    virtual iCustomTypeEditor *CreateEditor(QWidget *Parent) const = 0;
    virtual std::string ToDisplayText(const VariantArgumentMap::VariantType &in) const = 0;
};

template <class EDITOR>
struct CustomTypeEditorFactory : public iCustomTypeEditorFactory {
    virtual iCustomTypeEditor *CreateEditor(QWidget *Parent) const override { return new EDITOR(Parent); };
    virtual std::string ToDisplayText(const VariantArgumentMap::VariantType &in) const override {
        return EDITOR::ToDisplayText(in);
    }
};

class iCustomTypeEditorProvider {
public:
    virtual ~iCustomTypeEditorProvider() = default;

    virtual void RegisterTypeEditor(std::shared_ptr<iCustomTypeEditorFactory> factory,
                                    const std::string &type_name) = 0;
    virtual std::shared_ptr<iCustomTypeEditorFactory> GetEditorFactory(const std::string &type_name) = 0;

    template <typename EditorClass, typename T>
    void RegisterEditor() {
        RegisterTypeEditor(std::make_shared<CustomTypeEditorFactory<EditorClass>>(), typeid(T).name());
    }
    template <typename EditorClass>
    void RegisterEditor(const std::string &type_name) {
        RegisterTypeEditor(std::make_shared<CustomTypeEditorFactory<EditorClass>>(), type_name);
    }
};

} // namespace MoonGlare::Tools

Q_DECLARE_METATYPE(std::any)

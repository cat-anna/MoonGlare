
#pragma once

#include "CustomType.h"
#include "../iCustomEnum.h"

namespace MoonGlare::TypeEditor {

class CustomEnum : public CustomTypeEditor, public QComboBox {
public:
    CustomEnum(QWidget *Parent, std::shared_ptr<QtShared::iCustomEnum> Enum);

    void SetValue(const std::string &in) override;
    std::string GetValue()override;
protected:
    void showPopup() override;
private:
    std::shared_ptr<QtShared::iCustomEnum> customEnum;
};

} //namespace MoonGlare::TypeEditor

#include PCH_HEADER

#include "CustomEnum.h"

namespace MoonGlare::TypeEditor {

CustomEnum::CustomEnum(QWidget *Parent, std::shared_ptr<QtShared::iCustomEnum> Enum) : QComboBox(Parent), customEnum(std::move(Enum)) {
    for (auto &item : customEnum->GetValues()) {
        addItem(item.caption.c_str());
    }

    setInsertPolicy(QComboBox::NoInsert);
    model()->sort(0);

    //setEditable(true);
    //setInsertPolicy(QComboBox::InsertAtTop);
}

void CustomEnum::SetValue(const std::string &in) {
    setCurrentText(in.c_str());
}

std::string CustomEnum::GetValue() {
    return currentText().toLocal8Bit().constData();
}

} //namespace MoonGlare::TypeEditor 

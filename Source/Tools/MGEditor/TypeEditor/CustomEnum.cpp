#include PCH_HEADER

#include <qabstractitemview.h>

#include "CustomEnum.h"

namespace MoonGlare::TypeEditor {

CustomEnum::CustomEnum(QWidget *Parent, std::shared_ptr<QtShared::iCustomEnum> Enum) : QComboBox(Parent), customEnum(std::move(Enum)) {
    for (auto &item : customEnum->GetValues()) {
        addItem(item.caption.c_str());
    }

    setInsertPolicy(QComboBox::NoInsert);
    model()->sort(0);
    setEditable(false);
}

void fixComboBoxDropDownListSizeAdjustemnt(QComboBox *cb) {
    int scroll = cb->count() <= cb->maxVisibleItems() ? 0 :
        QApplication::style()->pixelMetric(QStyle::PixelMetric::PM_ScrollBarExtent);

    int max = 0;

    for (int i = 0; i < cb->count(); i++)
    {
        int width = cb->view()->fontMetrics().width(cb->itemText(i));
        if (max < width)max = width;
    }

    cb->view()->setMinimumWidth(scroll + max);
}

void CustomEnum::showPopup() {
    fixComboBoxDropDownListSizeAdjustemnt(this);
    QComboBox::showPopup();
}

void CustomEnum::SetValue(const std::string &in) {
    setCurrentText(in.c_str());
}

std::string CustomEnum::GetValue() {
    return currentText().toLocal8Bit().constData();
}

} //namespace MoonGlare::TypeEditor 

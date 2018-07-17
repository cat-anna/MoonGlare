#pragma once

namespace MoonGlare::TypeEditor {

#ifdef _X2C_IMPLEMENTATION_
template<typename ENUM>
class X2CEnumTemplate : public CustomTypeEditor, public QComboBox {
public:
    X2CEnumTemplate(QWidget *Parent) : QComboBox(Parent) {
        if (ENUM::GetValues(m_Values)) {
            for (auto &it : m_Values) {
                addItem(it.first.c_str(), it.second);
            }
            setInsertPolicy(QComboBox::NoInsert);
            model()->sort(0);
        }
        else {
            setEditable(true);
            setInsertPolicy(QComboBox::InsertAtTop);
        }
    }

    static std::string ToDisplayText(const std::string &in) {
        decltype(m_Values) Values;
        if (ENUM::GetValues(Values)) {
            for (auto &it : Values) {
                if (std::to_string(it.second) == in) {
                    return it.first;
                }
            }
        }
        return in;
    }

    virtual void SetValue(const std::string &in) {
        for (auto &it : m_Values) {
            if (std::to_string(it.second) == in) {
                setCurrentText(it.first.c_str());
                return;
            }
        }
        setCurrentText(in.c_str());
    }
    virtual std::string GetValue() {
        auto cdata = currentData();
        if (!cdata.isValid()) {
            return currentText().toLocal8Bit().constData();
        }
        bool succ;
        auto value = cdata.toULongLong(&succ);
        if (succ) {
            return std::to_string(value);
        }
        return currentText().toLocal8Bit().constData();
    }
private:
    std::unordered_map<std::string, uint64_t> m_Values;
};
#endif

} //namespace MoonGlare::TypeEditor

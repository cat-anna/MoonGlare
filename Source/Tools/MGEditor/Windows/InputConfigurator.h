#pragma once

#include <qtUtils.h>
#include <SubDialog.h>

namespace Ui { class InputConfigurator; }

namespace MoonGlare {
namespace Editor {

class InputConfigurator
    : public QtShared::SubDialog
    , public QtShared::UserQuestions
{
    Q_OBJECT
public:
    InputConfigurator(QWidget *parent, QtShared::SharedModuleManager ModuleManager, std::string configFileName);
    ~InputConfigurator();

    struct CustomItemDelegate;
private:
    std::unique_ptr<Ui::InputConfigurator> ui;
    std::unique_ptr<QStandardItemModel> model;

    std::string configFileName;

    void Load();
    void Save();
    void LoadDefaultConfig();

    struct DataModel;
    std::unique_ptr<DataModel> dataModel;
protected slots:
    void AcceptChanges();
    void Refresh();
    void InputAdd();
    void InputRemove();

    void ShowContextMenu(const QPoint &point);
    void ItemDoubleClicked(const QModelIndex&);
};

} //namespace Editor
} //namespace MoonGlare


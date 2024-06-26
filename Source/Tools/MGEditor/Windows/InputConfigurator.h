#pragma once

#include <SubDialog.h>
#include <ToolBase/UserQuestions.h>

namespace Ui { class InputConfigurator; }

namespace MoonGlare {
namespace Editor {

class InputConfigurator
    : public QtShared::SubDialog
    , public UserQuestions
{
    Q_OBJECT
public:
    InputConfigurator(QWidget *parent, SharedModuleManager ModuleManager, std::string configFileName);
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


#pragma once

#include <unordered_set>

#include <qwidget.h>

#include <Filesystem.h>
#include <ToolBase/Module.h>
#include <ToolBase/Modules/ChangesManager.h>
#include <ToolBase/UserQuestions.h>
#include <iEditor.h>

#include <Foundation/SoundSystem/iSoundSystem.h>
#include <ToolBase/interfaces/MainWindowTabs.h>

namespace Ui { class StringTableEditor; }

namespace MoonGlare::Editor {

class StringTableEditor
    : public QWidget
    , public QtShared::iEditor
    , public iChangeContainer 
    , public iTabViewBase
    , public UserQuestions
{
	Q_OBJECT;
public:
    StringTableEditor(QWidget *parent, SharedModuleManager smm, std::string tableName);
 	virtual ~StringTableEditor();

    virtual bool OpenData(const std::string &URI) {
        return true;
    }

public slots:
protected slots:    
private: 
	std::unique_ptr<Ui::StringTableEditor> ui;
    SharedModuleManager sharedModuleManager;
    std::shared_ptr<FileSystem> fileSystem;
    std::string tableName;

    struct LangInfo {
        std::string title;
        std::string suffix;
        int column;
        bool exists = false;
    };

    std::vector<LangInfo> languages;
    std::unique_ptr<QStandardItemModel> itemModel;

    void Reload();

// iTabViewBase
    std::string GetTabTitle() const override;
    bool CanClose() const override;

//iChangeContainer
    bool SaveChanges() override;
};

} 

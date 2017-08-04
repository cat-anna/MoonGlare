/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#include <DockWindow.h>
#include <iOutput.h>

namespace Ui { class OutputDockTab; }

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

class OutputDock;

class OutputTabWidget : public QWidget, public QtShared::iOutput {
    Q_OBJECT;
public:
    OutputTabWidget(OutputDock *parent);
    virtual ~OutputTabWidget();

    void PushLine(const std::string &line) override;
protected:
    OutputDock *owner;
    std::unique_ptr<Ui::OutputDockTab> m_Ui;
    std::unique_ptr<QStandardItemModel> m_ViewModel;
};                   


} //namespace DockWindows 
} //namespace Editor 
} //namespace MoonGlare 

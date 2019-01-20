/*
  * Generated by cppsrc.sh
  * On 2016-09-11 17:09:28,63
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef MiscIfs_H
#define MiscIfs_H

namespace MoonGlare {
namespace QtShared {
                                 
struct MainWindowProvider {
    virtual ~MainWindowProvider() {}
    virtual QWidget *GetMainWindowWidget() = 0;
};

template<typename T>
struct QtWindowProvider {
    virtual ~QtWindowProvider() {}
    virtual T *GetWindow() {
        return dynamic_cast<T*>(this);
    }
};

} //namespace QtShared
} //namespace MoonGlare

#endif

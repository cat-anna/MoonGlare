
#pragma once

#include <QMainWindow>

namespace MoonGlare::Tools {

struct MainWindowProvider {
    virtual ~MainWindowProvider() {}
    virtual QMainWindow *GetMainWindowWidget() = 0;
};

template <typename T> struct QtWindowProvider {
    virtual ~QtWindowProvider() {}
    virtual T *GetWindow() { return dynamic_cast<T *>(this); }
};

} // namespace MoonGlare::Tools

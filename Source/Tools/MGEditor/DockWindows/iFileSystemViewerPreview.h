#pragma once
    
namespace MoonGlare::Editor::DockWindows {
     
class iFileSystemViewerPreview {
public:
    virtual void SetPreviewEditor(QtShared::SharedEditor editor) = 0;
};

} 

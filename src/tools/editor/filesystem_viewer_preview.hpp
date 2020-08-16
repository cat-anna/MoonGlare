#pragma once

#include "editor_provider.h"

namespace MoonGlare::Tools::Editor {

class iFileSystemViewerPreview {
public:
    virtual void SetPreviewEditor(SharedEditor editor) = 0;

protected:
    virtual ~iFileSystemViewerPreview() = default;
};

} // namespace MoonGlare::Tools::Editor

#pragma once

namespace MoonGlare::DataClasses {

    namespace Fonts {
        class iFont;
        using Font = iFont;
        using FontPtr = std::shared_ptr<iFont>;
    }
    using Fonts::FontPtr;

} //namespace MoonGlare::DataClasses

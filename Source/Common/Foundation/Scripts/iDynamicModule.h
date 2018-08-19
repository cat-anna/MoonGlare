#pragma once

#include <boost/noncopyable.hpp>

namespace MoonGlare::Scripts {

class iDynamicScriptModule : private boost::noncopyable {
public:
    virtual ~iDynamicScriptModule() {}
};

} //namespace MoonGlare::Scripts

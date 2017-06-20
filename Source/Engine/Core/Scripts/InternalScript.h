#pragma once

namespace MoonGlare::Core::Scripts::InitCode {

static constexpr char *InitRequire = R"===(

--local RequireStorage = { }
local RequireQuerry = __RequireQuerry
__RequireQuerry = nil

function require(what)
    --local r = RequireStorage[what]
    --if r then
    --    return r
    --end

    r = RequireQuerry(what)
    if not r then
        error("Cannot find required " .. what, 2)
    end
    --RequireStorage[what] = r
    return r
end

)===";

}


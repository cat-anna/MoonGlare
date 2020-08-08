
require("premake-qt/qt")

local function enable_qt()
    premake.extensions.qt.enable()

    qtprefix "Qt5"
    qtmodules { "core", "gui", "widgets", "network", "charts"}

    -- filter "platforms:x32"
        -- qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
    -- filter "platforms:x64"
        -- qtpath(MoonGlare.GetBuildSetting({name = "qtPath_x64", group="Qt"}))
    -- filter {}
end

MoonGlare.feature_list.qt = enable_qt

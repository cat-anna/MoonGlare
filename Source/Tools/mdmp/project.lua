
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
	project "mdmp"
		filter { }

		kind "ConsoleApp"
		defines {
			"_BUILDING_TOOL_",
		}
		files {
			"**",
		}
		includedirs {
			".",
		}
		links {
            "ToolBase",
		}

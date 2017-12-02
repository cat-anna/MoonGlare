

group "Tools"
	project "rdcc"
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
		}

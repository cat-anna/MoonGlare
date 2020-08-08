tool_project "mdmp"
	filter { }

	kind "ConsoleApp"

	files {
		"**",
	}
	includedirs {
		".",
	}
	links {
		"tool_base",
	}

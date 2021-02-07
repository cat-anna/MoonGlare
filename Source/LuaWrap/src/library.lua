group "Libraries"

	project "LuaWrap"
		kind "StaticLib"
		location(dir.bin .. "/Libraries/LuaWrap")

		files {
			"**",
		}
		includedirs {
			".",
		}
		links {
		}

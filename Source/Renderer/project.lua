
group ""

	project "Renderer"
		kind "StaticLib"

		SetPCH { hdr = "pch.h", src = "pch.cpp", }

		defines {
			"_BUILDING_ENGINE_",
			"BT_EULER_DEFAULT_ZYX",
			"_FEATURE_EXTENDED_PERF_COUNTERS_",
		}

		files {
			"**",
			"../Config/*.h",
			"../Utils/**.h",
		}
		includedirs {
			".",
			"%{cfg.objdir}",
		}
		--links {
		--	"libSpace",
		--	"OrbitLogger",
		--	"StarVFS",
		--	"lua51jit",
		--	"freeimage",
		--	"glfx",
		--}

--		MoonGlare.X2CRule()
		--local fs = os.matchfiles("../Shared/x2c/**")
		--MoonGlare.X2C.Files(fs)
		--files {
		--	dir.bin .. "Engine/X2C.Process.lua",
		--}
		--local f = dir.bin .. "/Engine/X2C.Process.lua"
		--os.execute("echo  > " .. f)

		--for i,v in ipairs(fs) do
		--	local p = path.getabsolute(v)
		--	print(p)
		--	os.execute("echo " .. p .. " >> " .. f)
		--end

		filter "configurations:Debug"
			defines{

            }
		filter "configurations:Release"
			defines {

            }

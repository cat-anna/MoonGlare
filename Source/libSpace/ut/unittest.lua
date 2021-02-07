
group ""
		
	project "ut_libspace"
		kind "ConsoleApp"
		location(dir.bin .. "/ut/libspace")
		
		files {
			"**",
			"../src/**"
		}
		includedirs {
			".",
		}
		links {
			"gtest",
			"gtest_main",
		}
		debugargs { 
			"--gtest_break_on_failure" 
		}

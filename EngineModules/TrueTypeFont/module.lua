-- Moonglare module build script

define_module { 
	name = "TrueTypeFont",
	group = "Engine",
	
	Add = function()
		files { "*", }
		
		links { "freetype255MT", }
	end,

	--libs = { 
	--	release = { "" },
	--	debug = {  "freetype255MTd" },
	--	common = { },
	--},

}

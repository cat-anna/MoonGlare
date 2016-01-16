-- Moonglare module build script

define_module { 
	name = "BassSound",
	group = "Engine",
	
	Add = function()
		files { "*", }
		links { "bass", }
	end,
}

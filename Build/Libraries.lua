
local function CloneLibrary(conf, libconf)
	print "Lib clonning is not implemeted"
	os.exit(1)
end

local function CheckLibraryStatus(conf, libconf)
	os.chdir(libconf.path)
	
	local result = { os.execute("git diff-index --quiet HEAD --") }
	
	local changed = result[#result]

	if changed == 1 then
		print(string.format("Library %s is CHANGED", libconf.name))
	end
end

local function ProcessLibrary(conf, libconf)
	--print("Processing lib: ".. libconf.name .. "   " .. conf.startpath)
	libconf.path = conf.path .. libconf.name

	if not os.isdir(libconf.path) then
		return CloneLibrary(conf, libconf)
	end
	
	return CheckLibraryStatus(conf, libconf)
end

function Libraries(config) 
	if not config then
		return
	end
	
	local conf = {
		path = config.path,
		startpath = os.getcwd() .. "/",
	}
	
	local i,v
	for i,v in ipairs(config.libs) do
		ProcessLibrary(conf, v)
	end
	
	os.chdir(conf.startpath)
end

function MoonGlare.CheckLibraries()

	include(dir.libsrc .. "LibrariesConfig.lua")
	
end

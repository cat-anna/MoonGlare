
local function execute(cmd)
--	print(cmd)
	io.flush(io.output())
	local file = io.popen(cmd)
	local l
	local ret = { }
	while true do 
		l = file:read "*l"
		if not l then
			break
		end
		ret[#ret + 1] = l
	end
	file:close()
	return ret
end

local function firstline(cmd)
	local r = execute(cmd)
	return r[1]
end


local function CloneLibrary(conf, libconf)
	os.chdir(conf.path)
	execute(string.format("git clone -b %s %s", libconf.branch, libconf.remotes["origin"]))
	
--	print "Lib clonning is not implemeted"
	--os.exit(1)
end

local function ProcessLibrary(conf, libconf)
	io.write(string.format("Checking lib: %-15s ", libconf.name .. "..."))
	libconf.path = conf.path .. libconf.name

	libconf.exists = os.isdir(libconf.path)

	if not libconf.exists then
		if libconf.cloneattempt then
			print "Clone attempt failed."
			print "Cannot prepare build system."
			os.exit(1)
		end
	
		libconf.cloneattempt = true
		print("Not exists. Clonning...")
		CloneLibrary(conf, libconf)
		return ProcessLibrary(conf, libconf)
	end
	
	os.chdir(libconf.path)
	
	local branch = firstline("git rev-parse --symbolic-full-name --abbrev-ref HEAD")
	local commithash = firstline("git rev-parse HEAD")
	
	print (string.format("Status: %s, branch: %s, commit:%s", "ok", branch, commithash))
	
	--local result = { os.execute("git diff --cached --exit-code") }
	--local changed = result[#result]
--	if changed == 1 then
--		print(string.format("Library %s is CHANGED", libconf.name))
--	end

end

function Libraries(config) 
	if not config then
		return
	end
	
	local conf = {
		path = dir.libsrc,
		startpath = os.getcwd() .. "/",
	}
	
	local i,v
	for i,v in ipairs(config.libs) do
		ProcessLibrary(conf, v)
	end
	
	os.chdir(conf.startpath)
end

function MoonGlare.CheckLibraries()
	include(dir.libsrc .. "LibConfig.lua")
end

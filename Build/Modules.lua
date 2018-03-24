--Moonglare build subscript
--Engine modules build script generator

local ModuleManager_t = { }
ModuleManager_t.__index = ModuleManager_t

local CreateModuleManager = function ()
    local t = { }
    setmetatable(t, ModuleManager_t)
    t.Groups = { }
    t.Settings = { }
    return t;
end

ModuleManager = CreateModuleManager()

local DoAddModule = false

function define_module(data) 
	if DoAddModule then
		filter { }
		if not data.Add then
			ModuleManager:fail(string.format("Module %s:%s does not have valid add function", data.group, data.name))
		end	
		data.Add()
		DoAddModule = false
		return
	end

	return ModuleManager:RegisterModule(data)
end

-------------------

function ModuleManager_t:init()	
	self.Settings = MoonGlare.Settings.Modules
	if not self.Settings then
		self.Settings = {}
		MoonGlare.Settings.Modules = self.Settings
	end

    self:search()
end

function ModuleManager_t:fail(message)
	print(message)
	error(message)
	os.exit(1)
end;

function ModuleManager_t:search()
	local modlist = os.matchfiles(dir.base .. "**/module.lua")	
	for k,v in ipairs(modlist) do include(v) end
end

function ModuleManager_t:RegisterModule(data)
	data.script = _SCRIPT

	local modgroup = self.Groups[data.group]
	if not modgroup then
		modgroup = { }
		self.Groups[data.group] = modgroup
	end
	
	if modgroup[data.name] then
		self:fail("Error: module " .. data.name .. " allredy exists in group " .. data.group)
		return
	end

    modgroup[data.name] = data

    local settingsgroup = self.Settings[data.group]

    if not settingsgroup or settingsgroup[data.name] == nil then
        print(string.format("Found new module %s in group %s - Excluding from build", data.name, data.group))
        data.excluded = true
		if not settingsgroup then
			settingsgroup = { }
			self.Settings[data.group] = settingsgroup
		end
        settingsgroup[data.name] = false;
        return
    end

    if not settingsgroup[data.name] then
        data.excluded = true
		print(string.format("Found excluded module %s:%s", data.group, data.name))
    else
        data.excluded = false
		print(string.format("Found module %s:%s", data.group, data.name))
    end
end

function ModuleManager_t:AddModules(GroupName)
	local k,v
	local Group = self.Groups[GroupName]
	if not Group then
		print(string.format("Module group %s is empty!", GroupName))
		return 
	end
	for k,v in pairs(Group) do
		self:AddModule(v)
	end
end

function ModuleManager_t:AddModule(data)
    if not data or data.excluded then
        return
    end

	DoAddModule = true
	dofile(data.script)
end


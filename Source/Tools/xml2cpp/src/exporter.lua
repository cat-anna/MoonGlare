
local Exporter = { }
x2c.Classes.Exporter = Exporter

function Exporter:Init(Config)
	self.Config = Config
	self.Types = { }
end

function Exporter:Write()
  error("Not implemented!")
end

function Exporter:RegisterType(tinfo)
	self.Types[#self.Types + 1] = tinfo
end

function Exporter:GetObserver()
    local Observer = x2c.Classes.ObserverBroadcast.New()
	Observer:Add(self)
	return Observer
end

---------------------------------------

function Exporter:InitTypeExporterInfo(data)
  error("Not implemented!")
end

function Exporter:InitTypeExporterMemberInfo(data)
  error("Not implemented!")
end

---------------------------------------

function Exporter:MakeStructure(data)
  error("Not implemented!")
end

function Exporter:MakeAlias(data)
  error("Not implemented!")
end

function Exporter:MakeEnum(data)
  error("Not implemented!")
end

function Exporter:MakeContainer(data)
  error("Not implemented!")
end

---------------------------------------

function x2c.EnableExporter(class)
	if class.Enabled then
		return true
	end
	class.Enabled = true
	local o = class:GetObserver()
	if not o then
		error("Failed to create observer for ", class.Name)
	end
	x2c.observers[#x2c.observers + 1] = o
	return true
end

function x2c.RegisterExporter(class, name)
	x2c.exporters[name] = class
	class.Enabled = false
	class.Name = name
	local arg = "--enable-" .. name
	x2c.ArgumentsTable[arg] = {
		Help = "Enable " .. name .. " exporter",
		func = function()
			x2c.EnableExporter(class)
			return 0
		end,
	}
end

---------------------------------------

require "exporters/cxx-common"
require "exporters/cxxpugi"

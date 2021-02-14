#!/usr/bin/lua

--[[

%%pre-namespace-entry
namespace x2c::n1::n2 {
%%post-namespace-entry



	%%pre-enum-definition
	enum Enum {
		a, b, c,
	};
	%%post-enum-definition

%%pre-namespace-exit
}
%%post-namespace-exit

]]

local function InitPackagePath()
	local ScriptPath = arg[0]:gsub("xml2cpp.lua", "")
	local PathSep = ScriptPath:sub(ScriptPath:len())

	print(PathSep)
	return table.concat({
		"",
		ScriptPath .. table.concat({ "?", "init.lua" }, PathSep),
		ScriptPath .. table.concat({ "?.lua" }, PathSep),
	}, ";")
end

package.path = package.path .. InitPackagePath()

x2c = {
	inputfiles = { },
	outputfile = nil,

	settings = {
		gen_all = false,
	},

    exporters = { },

	imports = { },
	importsByName = { },
	importsByLevel = { },

	Classes = { },

    exports = { },

	observers = { },

	info = {
		Name = "xml2cpp",
		LongName = "xml2cpp v0.1",
	},
	detail = { },

	ExportTypes = { },
}

require "utils"
require "basetype"
require "import"
require "state"
require "arguments"
require "observer"
require "namespace"
require "structure"
require "enum"
require "alias"
require "container"
require "writter"
require "exporter"

x2c.Generators = require "cxx"

local GenPoints = require("observer").GenPoints

local GeneratorFrontend

local function CallObservers(point, type, block, ...)
	for _,v in ipairs(x2c.observers) do
		local f = v[point]
		if f then
			f(v, type, block:AddChildBlock(), ...)
		end
	end
end

 GeneratorFrontend = setmetatable({}, {
	__index = function(self, name)
		if name == "Call" then
			return function(self, point, type, block)
				return CallObservers(point, type, block, GeneratorFrontend)
			end
		end

		return function(self, t, block, ...)
			local typename = t:GeneratorType()
			local gen = x2c.Generators[typename]
			if not gen then
				error("Unknown generator: ", typename)
			end

			local f = gen[name]
			if not f then
				error("Generator ", typename, " does not support action ", name)
			end

			--print("Execute generator: ", typename .. ":" .. name, " for ", t)

			return f(gen, t, block, GeneratorFrontend, ...)
		end
	end
})

function x2c.detail.WriteImplementation(t, block, ...)

	local typename = t:Type()
	local gen = x2c.Generators[typename]
	if not gen then
		error("Unknown generator: ", typename)
	end

	GeneratorFrontend:GenerateType(t, block, frontend, ...)
end

function x2c.detail.WriteTypes(writter, block)
	local namespace = nil

	for i,v in ipairs(x2c.ExportTypes) do
		--print(v.value:GetName())
		local t = v.value
		local tnamespace = t.namespace

		if tnamespace ~= namespace then
			if namespace then
				namespace:WriteLeave(block:AddChildBlock())
			end

			namespace = tnamespace
			namespace:WriteEnter(block:AddChildBlock())
		end

		block:BeginBlock()
		GeneratorFrontend:GenerateType(t, block)
		block:EndBlock()
	end

	if namespace then
		namespace:WriteLeave(block)
	end
end

function x2c.detail.Write()
	local writter = x2c.Classes.CXXWritter:Create(x2c.outputfile)
--	self.ImplWritter = x2c.Classes.CXXWritter:Create(self.Config.ImplFile)

	writter:WriteFileHeader()
--	self.ImplWritter:WriteFileHeader()

--	self.ImplWritter:WriteX2CImpl(self.ImplList)

--	local incblock = self.Writter:AddFileBlock()

	--local cf =

	CallObservers(GenPoints.file_start, nil, writter:AddFileBlock("start"))

	x2c.detail.WriteTypes(writter, writter:AddFileBlock("content"))

	CallObservers(GenPoints.file_end, nil, writter:AddFileBlock("end"))
--	incblock:IncludeLocal("x2c.h")
--	if not x2c.settings.gen_all then
--		for i,v in ipairs(x2c.importsByLevel[1] or {}) do
--			incblock:IncludeLocal(v.FileName .. ".h")
--		end
--	end
--	incblock:Line ""

--	self:WriteX2CImpl(self.Writter:AddFileBlock())

    writter:Close()
--	self.ImplWritter:Close()
end

x2c.ParseArguments(arg)
x2c.CheckStartingState()
x2c.ImportInputFiles()
x2c.detail.Write()

--local basedir = "d:\\Programowanie\\Projekty\\!gry\\MoonGlare\\MazeGame\\"
--x2c.inputfiles[#x2c.inputfiles + 1] =  basedir .. "MoonGlare\\Source\\Shared\\x2c\\Math.x2c"
--x2c.outputfile = {
--  exporter = "cxxpugi",
--  FileName = basedir .. "bin_vs2015\\Engine\\obj\\x32\\Debug\\Math.x2c.h",
--}

---------------

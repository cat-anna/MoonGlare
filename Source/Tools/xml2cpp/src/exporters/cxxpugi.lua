local CXXPugi = inheritsFrom(x2c.Classes.Exporter)

------------------------------------------------------------------------

local StrucType, StrucTypeMt = x2c.MakeTypeClass()

function StrucType:LocalName()
	return self.classname
end

function StrucType:GlobalName()
	return self.namespace:GlobalName() .. "::" .. self.classname
end

function CXXPugi:MakeStructure(data)
	return setmetatable(data, StrucTypeMt)
end

------------------------------------------------------------------------

local CXXPugiTypes = require "exporters/cxxpugi-types"

------------------------------------------------------------------------

local Alias_t, Alias_t_mt = x2c.MakeTypeClass()

function Alias_t:GenResetToDefault(member, name, writter, ...)
	self.source_type:GenResetToDefault(member, name, writter, ...)
end

function Alias_t:GenWrite(member, name, writter)
	self.source_type:GenWrite(member, name, writter)
end

function Alias_t:GenRead(member, name, writter)
	self.source_type:GenRead(member, name, writter)
end

function Alias_t:GetBaseType()
	if not self.source_type or not self.source_type.GetBaseType then
		return self
	end
	return self.source_type:GetBaseType()
end

function CXXPugi:MakeAlias(data)
	setmetatable(data, Alias_t_mt)
	return data
end

------------------------------------------------------------------------

local EnumType, EnumTypeMt = x2c.MakeTypeClass()

function EnumType:GenResetToDefault(member, name, block)
	error()
end

function EnumType:GetDefault()
	return self.default
end

function CXXPugi:MakeEnum(data)
	return setmetatable(data, EnumTypeMt)
end

------------------------------------------------------------------------

local ContainerTypeMt = require "exporters/cxxpugi-containers"

function CXXPugi:MakeContainer(data)
	return setmetatable(data, ContainerTypeMt)
end

------------------------------------------------------------------------

function CXXPugi:Init(Config)
    self.Base.Init(self, Config)
    self.ImplList = CXXPugiTypes

	for k,v in pairs(CXXPugiTypes) do
		x2c.GlobalNamespace:Add(v)
	end
end

function CXXPugi:InitTypeExporterInfo(data)
    data.exportsettings = data.pugi or { }
end

function CXXPugi:InitTypeExporterMemberInfo(data)
    if not data.pugi then
        data.pugi = { }
    end

    data.exportsettings = data.pugi
end

function CXXPugi:GetObserver()
	local Observer = x2c.Classes.ObserverBroadcast.New()
	Observer:Add(require "cxx/exporters/pugi-structure")
	Observer:Add(require "cxx/exporters/pugi-enum")
	Observer:Add(require "cxx/exporters/pugi-container")
	Observer:Add(self)
	return Observer
end

function CXXPugi:file_start(_, Block)
	local StartBlock = Block:GetWritter():FindFileBlock("start")
	StartBlock:IncludeLocal("x2c.h")
end

function CXXPugi:file_end(_, Block)
	self:WriteX2CImpl()
end

function CXXPugi:WriteX2CImpl()
	local ImplFile

	print(x2c.outputfile)
	ImplFile = (x2c.outputfile:match("(.*[/\\])") or "") .. "x2c.h"

	local writter = x2c.Classes.CXXWritter:Create(ImplFile)
	writter:WriteFileHeader()
	local block = writter:AddFileBlock()

	block:Line { "#ifndef _X2C_IMPLEMENTATION_" }
	block:Line { "#define _X2C_IMPLEMENTATION_" }
	block:Line ""
	block:BeginNamespace("x2c")
	block:BeginNamespace("cxxpugi")

	block:Line { "template<bool Required, bool Attribute, class Type, class Reader>" }
	block:Line { "inline bool Read(Type &, Reader, const char *);"}

	block:Line { "template<bool Attribute, class Type, class Reader>" }
	block:Line { "inline bool Write(const Type &, Reader, const char *);"}

	block:Line { }

	local first = true
	for i,v in pairs(self.ImplList) do
		if not first then
			block:Line { }
		end
		first = false
		v:GenImplementation(block)
	end

	block:Line { }

	block:Line { "template<typename ENUM>" }
	block:BeginBlockLine { "void EnumToString(ENUM e, std::string &out) {" }
	block:Line { "out = std::to_string(static_cast<uint64_t>(e));" }
	block:EndBlockLine "}"
	block:Line { "template<typename ENUM>" }
	block:BeginBlockLine { "void StringToEnum(const std::string &in, ENUM &e) {" }
	block:Line { "e = static_cast<ENUM>(::strtoull(in.c_str(), nullptr, 10));" }
	block:EndBlockLine "}"

	block:Line { }

	block:BeginStructure("StructureMemberInfo")
	block:Line "std::string m_Name;"
	block:Line "std::string m_Default;"
	block:Line "std::string m_Description;"
	block:Line "std::string m_TypeName;"
	block:EndStructure()
	block:MakeAlias("StructureMemberInfoTable", "std::vector<StructureMemberInfo>")

	block:EndNamespace()
	block:EndNamespace()
	block:Line ""
	block:Line { "#endif //_X2C_IMPLEMENTATION_" }

	writter:Close()
end

------------------------------------------------------------------------

x2c.Exporter = CXXPugi:Create()
x2c.RegisterExporter(x2c.Exporter, "cxxpugi")

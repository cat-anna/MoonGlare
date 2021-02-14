
local Assert = x2c.Assert

-----------------------

local BaseType, BaseTypeMt = x2c.MakeTypeClass()

---------------------------------------

function BaseType:GenImplementation(block)

	local Read = function (req, attr)
		block:Line { "template<>" }
		block:BeginBlockLine { 
			"inline bool Read<", 
			IfThen(req, "true", "false"), ", ", 
			IfThen(attr, "true", "false"), 
			", ", self:LocalName(), ", pugi::xml_node>(", 
				self:LocalName(), " &value, pugi::xml_node node, const char *name) {", }	
	end
	
	
	local Write = function (attr)
		block:Line { "template<>" }
		block:BeginBlockLine { 
			"inline bool Write<", 
			IfThen(attr, "true", "false"), 
			", ", self:LocalName(), ", pugi::xml_node>(", 
				"const ", self:LocalName(), " &value, pugi::xml_node node, const char *name) {", }	
	end	
	
	local ReadCast = string.format(self.ReadCast or "", self:LocalName())
	local WriteCast = string.format(self.WriteCast or "value", self:LocalName())

	Read(false, false)
	block:Line { "auto item = name ? node.child(name) : node;", }
	block:Line { "if(item) value = ", ReadCast, "(item.text().", self.pugi_read, "(", WriteCast, "));", }
	block:Line { "return true;" }
	block:EndBlockLine "}"
	
	Read(true, false)
	block:Line { "auto item = name ? node.child(name) : node;", }
	block:Line { "if(!item) return false;", }
	block:Line { "value = ", ReadCast, "(item.text().", self.pugi_read, "(", WriteCast, "));", }
	block:Line { "return true;" }
	block:EndBlockLine "}"

	Read(false, true)
	block:Line { "if(!name) return false;", }
	block:Line { "auto item = node.attribute(name);", }
	block:Line { "if(item) value = ", ReadCast, "(item.", self.pugi_read, "(", WriteCast, "));", }
	block:Line { "return true;" }	
	block:EndBlockLine "}"
	
	Read(true, true)
	block:Line { "if(!name) return false;", }
	block:Line { "auto item = node.attribute(name);", }
	block:Line { "if(!item) return false;", }
	block:Line { "value = ", ReadCast, "(item.", self.pugi_read, "(", WriteCast, "));", }
	block:Line { "return true;" }
	block:EndBlockLine "}"
	
	
	Write(false)
	block:BeginBlockLine "if(name) {"
	block:Line { "auto item = node.child(name);", }
	block:Line { "node = item ? item : node.append_child(name);", }
	block:EndBlockLine "}"
	block:Line { "node.text() = ", WriteCast, ";", }
	block:Line { "return true;" }
	block:EndBlockLine "}"

	Write(true)
	block:Line { "if(!name) name = \"Value\";", }
	block:Line { "auto item = node.attribute(name);", }
	block:Line { "if(!item) item = node.append_attribute(name);", }
	block:Line { "item = ", WriteCast, ";", }
	block:Line { "return true;" }
	block:EndBlockLine "}"
end

function BaseType:GenResetToDefault(member, name, writter, exportsettings, default)
	exportsettings = exportsettings or { }
	local value
	default = default or self:GetDefault()
	
	if self.AssignCast then
		value = string.format(self.AssignCast, tostring(default))
	else 
		value = string.format("static_cast<%s>(%s)", self:LocalName(), tostring(default))
	end

	writter:Line { member, " = ", value, ";", }
end

function BaseType:GenWrite(member, name, writter, exportsettings)
	exportsettings = exportsettings or { }
	local attr = IfThen(exportsettings.useattribute, "true", "false")
	if exportsettings.alias then
		name = "\"" .. exportsettings.alias .. "\""
	end	
	writter:Line { "if(!::x2c::cxxpugi::Write<", attr, ">(", member, ", node, ", name, ")) return false;"}
end

function BaseType:GenRead(member, name, writter, exportsettings, default)
	exportsettings = exportsettings or { }
	local attr = IfThen(exportsettings.useattribute, "true", "false")
	if exportsettings.alias then
		name = "\"" .. exportsettings.alias .. "\""
	end	
	local req = IfThen(exportsettings.require, "true", "false")
	writter:Line { "if(!::x2c::cxxpugi::Read<", req, ", ", attr, ">(", member, ", node, ", name, ")) return false;"}	
end

---------------------------------------

function BaseType:GlobalName()
	return self:LocalName()
end

function BaseType:GetDefault()
	return self.default
end

---------------------------------------

local TypesMeta = { }
local TypesList = { }

local function make_type(t)
	t.object_type = "Type"
	t.namespace = x2c.GlobalNamespace
	setmetatable(t, BaseTypeMt)
	TypesMeta[t:GetName()] = t
	TypesList[t:GetName()] = t
end

local function make_intergral(t)
	t.integral = true
	t.default = "0"
	return make_type(t)
end

---------------------------------------

make_intergral { name = "s8",  local_name =   "int8_t", pugi_read="as_int",    ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "u8",  local_name =  "uint8_t", pugi_read="as_uint",   ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "s16", local_name =  "int16_t", pugi_read="as_int",    ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "u16", local_name = "uint16_t", pugi_read="as_uint",   ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "s32", local_name =  "int32_t", pugi_read="as_int",    ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "u32", local_name = "uint32_t", pugi_read="as_uint",   ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "s64", local_name =  "int64_t", pugi_read="as_llong",  ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }
make_intergral { name = "u64", local_name = "uint64_t", pugi_read="as_ullong", ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)" }

make_type { name = "float",  local_name = "float",       default = "0.0f",  pugi_read="as_float",  ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)", }
make_type { name = "double", local_name = "double",      default = "0.0",   pugi_read="as_double", ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)", }
make_type { name = "bool",   local_name = "bool",        default = "false", pugi_read="as_bool" ,  ReadCast="static_cast<%s>", WriteCast="static_cast<%s>(value)", }
make_type { name = "string", local_name = "std::string", default = "",      pugi_read="as_string", ReadCast="std::string",     WriteCast="value.c_str()", AssignCast="\"%s\"", }

x2c.MakeMetaObject(TypesMeta, "Types")

return TypesList

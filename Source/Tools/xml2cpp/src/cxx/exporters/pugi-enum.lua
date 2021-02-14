
local Enum = { }

function Enum:pre_enum(Type, Block, Generator)
end

function Enum:enum_import(Type, Block, Generator)
end

function Enum:enum_entry(Type, Block, Generator)
end

function Enum:enum_exit(Type, Block, Generator)
end

function Enum:post_enum(Type, Block, Generator)
	local attr = IfThen(Type.exportsettings.useattribute, "true", "false")

	Block:Line { "inline bool ", Type:LocalName(), "_Write(pugi::xml_node node, ", Type:LocalName(), " value, const char* name) {", }
	Block:BeginBlock()
	Block:Line { "uint64_t temp = static_cast<uint64_t>(value);", }
	Block:Line { "if(!::x2c::cxxpugi::Write<", attr, ">(temp, node, name)) return false;", }
	Block:Line { "return true;" }
	Block:EndBlock()
	Block:Line { "};", }

	Block:Line { "inline bool ", Type:LocalName(), "_Read(const pugi::xml_node node, ", Type:LocalName(), " &value, const char* name) {", }
	Block:BeginBlock()
	Block:Line { "uint64_t temp = static_cast<uint64_t>(value);", }
	Block:Line { "if(!::x2c::cxxpugi::Read<true, ", attr, ">(temp, node, name)) return false;", }
	Block:Line { "value = static_cast<", Type:LocalName(), ">(temp);", }
	Block:Line { "return true;" }
	Block:EndBlock()
	Block:Line { "};", }
end

------------------------------------------------------------------------------

return Enum

local Assert = x2c.Assert
---------------------------------------

local ContainerType, ContainerTypeMt = x2c.MakeTypeClass()

function ContainerType:WriteList(block)
	local element = self.element_type

	block:Line {
		"using ", self:LocalName(), " = std::list<", element:GlobalName(), ">;"
	}

	block:Line {
		"inline bool ",
		self:LocalName(),
		"_Write(pugi::xml_node in_node, const ",
		self:LocalName(),
		" &value, const char* name) {"
	}
	block:BeginBlock()
	block:Line "pugi::xml_node parent;"
	block:Line "if(name == nullptr)"
	block:DefBlockLine "parent = in_node;"
	block:Line "else {"
	block:BeginBlock()
	block:Line "parent = in_node.child(name);"
	block:Line "if(!parent) parent = in_node.append_child(name);"
	block:EndBlock()
	block:Line "}"
	block:Line "if(!parent) return false;"
	block:Line "while(parent.remove_child(parent.first_child()));"
	block:Line "for(const auto& it: value) {"
	block:BeginBlock()
	block:Line "auto node = parent.append_child(\"item\");"
	element:GenWrite("it", "\"item\"", block)
	block:EndBlock()
	block:Line "}"
	block:Line "auto count_att = parent.attribute(\"count\");"
	block:Line "if(!count_att) count_att = parent.append_attribute(\"count\") "
	block:Line "count_att = value.size();"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

	block:Line {
		"inline bool ",
		T:LocalName(),
		"_Read(const pugi::xml_node in_node, ",
		T:LocalName(),
		" &value, const char* name) {"
	}
	block:BeginBlock()
	block:Line "pugi::xml_node parent = name == nullptr ? in_node : in_node.child(name);"
	block:Line "if(!parent) return false;"
	block:Line "value.clear();"
	block:Line "for(auto node = parent.first_child(\"item\"); it; it = it.next_sibling(\"item\") {"
	block:BeginBlock()
	block:Line { element:GlobalName(), " item;" }
	element:GenRead("item", "\"item\"", block)
	block:Line "value.push_back(std::move(item));"
	block:EndBlock()
	block:Line "}"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

		block:Line {
		"inline bool ",
		self:LocalName(),
		"_SetDefault(",
		self:LocalName(),
		" &value) {"
	}
	block:BeginBlock()
	block:Line "value.clear();"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

	block:Line ""
end

function ContainerType:WriteMap(block)
	local element = self.element_type
	local key = self.key_type

	block:Line {
		"using ", self:LocalName(), " = std::unordered_map<", key:GlobalName(), ", ", element:GlobalName(), ">;"
	}

	block:Line {
		"inline bool ",
		self:LocalName(),
		"_Write(pugi::xml_node in_node, const ",
		self:LocalName(),
		" &value, const char* name) {"
	}
	block:BeginBlock()
	block:Line "pugi::xml_node parent;"
	block:Line "if(name == nullptr)"
	block:DefBlockLine "parent = in_node;"
	block:Line "else {"
	block:BeginBlock()
	block:Line "parent = in_node.child(name);"
	block:Line "if(!parent) parent = in_node.append_child(name);"
	block:EndBlock()
	block:Line "}"
	block:Line "if(!parent) return false;"
	block:Line "while(parent.remove_child(parent.first_child()));"
	block:Line "for(const auto& it: value) {"
	block:BeginBlock()
	block:Line "auto node = parent.append_child(\"item\");"
	key:GenWrite("it.first", "\"first\"", block)
	element:GenWrite("it.second", "\"second\"", block)
	block:EndBlock()
	block:Line "}"
	block:Line "auto count_att = parent.attribute(\"count\");"
	block:Line "if(!count_att) count_att = parent.append_attribute(\"count\");"
	block:Line "count_att = value.size();"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

	block:Line {
		"inline bool ",
		self:LocalName(),
		"_Read(const pugi::xml_node in_node, ",
		self:LocalName(),
		" &value, const char* name) {"
	}
	block:BeginBlock()
	block:Line "pugi::xml_node parent = name == nullptr ? in_node : in_node.child(name);"
	block:Line "if(!parent) return false;"
	block:Line "value.clear();"
	block:Line "auto count_att = parent.attribute(\"count\");"
	block:Line "if(!count_att) value.reserve(count_att.as_uint());"
	block:Line "for(auto node = parent.first_child(\"item\"); node; node = node.next_sibling(\"item\") {"
	block:BeginBlock()
	block:Line { key:GlobalName(), " first;" }
	block:Line { element:GlobalName(), " second;" }
	key:GenRead("first", "\"first\"", block)
	element:GenRead("second", "\"second\"", block)
	block:Line "value.insert(std::make_pair(std::move(first), std::move(second)));"
	block:EndBlock()
	block:Line "}"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

	block:Line {
		"inline bool ",
		self:LocalName(),
		"_SetDefault(",
		self:LocalName(),
		" &value) {"
	}
	block:BeginBlock()
	block:Line "value.clear();"
	block:Line "return true;"
	block:EndBlock()
	block:Line "}"

	block:Line ""
end

function ContainerType:WriteImplementation(block)
	local WriteFunc = "Write" .. self.ContainerType
	local Func = self[WriteFunc]
	if not Func then
		error("Invalid container type " .. self.ContainerType)
	end
	Func(self, block)
end


return ContainerTypeMt

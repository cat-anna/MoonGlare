
local Structure = { }

function Structure:post_struct_declartion(iType, Block, Generator)
	local Type = {
		LocalName = iType:LocalName(),
	}

	Block:Format ("inline bool {LocalName}_Write(pugi::xml_node in_node, const {LocalName} &value, const char* name);", Type)
	Block:Format ("inline bool {LocalName}_Read(const pugi::xml_node in_node, {LocalName} &value, const char* name);", Type)
end

function Structure:struct_public(Type, Block, Generator)
	local iType = {
		LocalName = Type:LocalName(),
		GlobalName = Type:GlobalName(),
	}

	Block:Format "bool Read(const pugi::xml_node node, const char *name = nullptr) {"
	Block:BlockFormat ( "return {GlobalName}_Read(node, *this, name);", iType)
	Block:Format "}"
	Block:Format "bool Write(pugi::xml_node node, const char *name = nullptr) const {"
	Block:BlockFormat ( "return {GlobalName}_Write(node, *this, name);", iType)
	Block:Format "}"

	Block:Line "bool WriteFile(const std::string& filename) const {"
	Block:BeginBlock()
	Block:Line "pugi::xml_document doc;"
	Block:Line {"auto root = doc.append_child(\"", Type:GetName(), "\");" }
	Block:Line "if(!Write(root)) return false;"
	Block:Line "return doc.save_file(filename.c_str());"
	Block:EndBlock()
	Block:Line "}"
	Block:Line "bool ReadFile(const std::string& filename) {"
	Block:BeginBlock()
	Block:Line "pugi::xml_document doc;"
	Block:Line "doc.load_file(filename.c_str());"
	Block:Line {"return Read(doc.child(\"", Type:GetName(), "\"));" }
	Block:EndBlock()
	Block:Line "}"
end

function Structure:post_struct(Type, Block, Generator)
	Block:Line { "inline bool ", Type:LocalName(), "_Read(const pugi::xml_node in_node, ", Type:LocalName(), " &value, const char* name) {", }
	Block:BeginBlock()
	Block:Line "pugi::xml_node node = name == nullptr ? in_node : in_node.child(name);"
	Block:Line "if(!node) return false;"
	for i,v in ipairs(Type.fields) do
		v.type:GenRead("value." .. v.decoratedname, quote(v.name), Block, v.exportsettings, v.default)
	end
	Block:Line "return true;"
	Block:EndBlock()
	Block:Line "}"

	Block:Line {
		"inline bool ",
		Type:LocalName(),
		"_Write(pugi::xml_node in_node, const ",
		Type:LocalName(),
		" &value, const char* name) {"
	}
	Block:BeginBlock()
	Block:Line "pugi::xml_node node;"
	Block:Line "if(name == nullptr)"
	Block:BlockLine "node = in_node;"
	Block:Line "else {"
	Block:BeginBlock()
	Block:Line "node = in_node.child(name);"
	Block:Line "if(!node) node = in_node.append_child(name);"
	Block:EndBlock()
	Block:Line "}"
	Block:Line "if(!node) return false;"
	for i,v in ipairs(Type.fields) do
		v.type:GenWrite("value." .. v.decoratedname, quote(v.name), Block, v.exportsettings)
	end
	Block:Line "return true;"
	Block:EndBlock()
	Block:Line "}"
end

------------------------------------------------------------------------

return Structure

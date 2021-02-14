
local Container = { }

function Container:post_table(Type, Block, Generator)
    local element = Type.element_type

    Block:BeginBlockLine { "inline bool ", Type:LocalName(), "_Write(pugi::xml_node in_node, const ", Type:LocalName(), " &value, const char* name) {", }
        Block:Line "pugi::xml_node parent;"
        Block:Line "if(name == nullptr)"
            Block:BlockLine "parent = in_node;"
        Block:Line "else {"
        Block:BeginBlock()
            Block:Line "parent = in_node.child(name);"
            Block:Line "if(!parent) parent = in_node.append_child(name);"
        Block:EndBlock()
        Block:Line "}"
        Block:Line "if(!parent) return false;"
        Block:Line "while(parent.remove_child(parent.first_child()));"
        Block:Line "for(const auto &it: value) {"
        Block:BeginBlock()
            Block:Line "auto node = parent.append_child(\"item\");"
            element:GenWrite("it", "nullptr", Block)
        Block:EndBlock()
        Block:Line "}"
        Block:Line "auto count_att = parent.attribute(\"count\");"
        Block:Line "if(!count_att) count_att = parent.append_attribute(\"count\");"
        Block:Line "count_att = value.size();"
        Block:Line "return true;"
    Block:EndBlockLine "}"

    Block:Line { "inline bool ", Type:LocalName(), "_Read(const pugi::xml_node in_node, ", Type:LocalName(), " &value, const char* name) {", }
    Block:BeginBlock()
        Block:Line "pugi::xml_node parent = name == nullptr ? in_node : in_node.child(name);"
        Block:Line "if(!parent) return false;"
        Block:Line "value.clear();"
        Block:Line "auto count_att = parent.attribute(\"count\");"
        Block:Line "if(count_att) value.reserve(count_att.as_uint());"
        Block:BeginBlockLine "for(auto node = parent.child(\"item\"); node; node = node.next_sibling(\"item\")) {"
            Block:Line { element:GlobalName(), " tableitem;" }
            element:GenResetToDefault("tableitem", nil, Block)
            element:GenRead("tableitem", "nullptr", Block)
            Block:Line "value.push_back(std::move(tableitem));"
        Block:EndBlockLine "}"
        Block:Line "return true;"
    Block:EndBlockLine "}"

    Block:Line ""

    Block:BeginBlockLine { "inline bool ", Type:LocalName(), "_SetDefault(", Type:LocalName(), " &value) {", }
        Block:Line "value.clear();"
        Block:Line "return true;"
    Block:EndBlockLine "}"
end

--[[
function Container:(Type, Block, Generator)
end
]]

------------------------------------------------------------------------------

return Container

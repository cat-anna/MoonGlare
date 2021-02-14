local GenPoints = require("observer").GenPoints

----------------------------------------------------------------------------------------------------

local Table = { }

function Table:GenerateType(Type, Block, Generator)
    local bcast = function(point)
        return Generator:Call(point, Type, Block)
    end

    local element = Type.element_type

    bcast(GenPoints.pre_table)
	Block:MakeAlias(Type:LocalName(), "std::vector<" .. element:GlobalName() .. ">")
    bcast(GenPoints.post_table)
end

function Table:GenereateReset(Type, Block, Generator, variable, default )--member, name, writter, exportsettings, default)
	local value
	Block:Line { variable, ".clear();", }
end

----------------------------------------------------------------------------------------------------

local List = { }

----------------------------------------------------------------------------------------------------

local Map = { }

----------------------------------------------------------------------------------------------------

return {
    Table = Table,
}

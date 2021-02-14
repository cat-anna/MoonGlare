local GenPoints = require("observer").GenPoints

local Alias = { }

function Alias:GenerateType(Type, Block, Generator)
    local bcast = function(point)
        return Generator:Call(point, Type, Block)
    end

    bcast(GenPoints.pre_alias)
    Block:MakeAlias(Type:LocalName(), Type.source_type:GlobalName())
    bcast(GenPoints.post_alias)
end

function Alias:GenereateReset(Type, Block, Generator, variable, default)
    Generator:GenereateReset(Type.source_type, Block, variable, default)
end

return x2c.MakeGenerator(Alias)

local GenPoints = require("observer").GenPoints

local Structure = { }

function Structure:GenerateType(Type, Block, Generator)
    local bcast = function(point)
        return Generator:Call(point, Type, Block:AddChildBlock())
    end

    if Type.imported then
        bcast(GenPoints.pre_struct_declartion)
        Block:DocString(Type.description)
        Block:MakeAlias(Type:LocalName(), Type.location)
        bcast(GenPoints.struct_import)
        bcast(GenPoints.post_struct_declartion)
        bcast(GenPoints.post_struct)
        return
    end

    bcast(GenPoints.pre_struct_declartion)
    Block:DefineStructure(Type:LocalName())

    bcast(GenPoints.post_struct_declartion)

    bcast(GenPoints.pre_struct_definition)

    Block:DocString(Type.description)
    Block:BeginStructure(Type:LocalName())
        bcast(GenPoints.struct_entry)
            --tbd
        bcast(GenPoints.pre_struct_members)
            --gen members

        if Type.fields then
    		for _,v in ipairs(Type.fields) do
    			Block:DocString(v.description)
    			Block:Line { v.type:GlobalName(), " ", v.decoratedname, ";", }
    		end
            Block:Line()
    	end

        bcast(GenPoints.post_struct_members)

        bcast(GenPoints.struct_public)

        bcast(GenPoints.struct_exit)
	Block:EndStructure()

    bcast(GenPoints.post_struct_definition)

    bcast(GenPoints.post_struct)
--[[
    %%pre_struct_declartion
    struct name;
    %%post_struct_declartion
    %%pre_struct_definition
    /** doc */
    struct name {
        %%struct_entry/%%struct_import
        %%pre_struct_members
        int members;
        %%post_struct_members
        %%struct_exit
    };
    %%post_struct_definition
]]
end

function Structure:GenereateReset(Type, Block, Generator, variable, default)
    default = default or { }
    for _,v in ipairs(Type.fields) do
        Generator:GenereateReset(v.type, Block, variable ..  "." .. v.decoratedname, default[v.name] or v.default)
    end
end


return x2c.MakeGenerator(Structure)

local GenPoints = require("observer").GenPoints

local Enum = { }

--[[
"pre_enum",
"enum_import",
"enum_entry",
"enum_exit",
"post_enum",
]]

function Enum:GenerateType(Type, Block, Generator)
    local bcast = function(point)
        return Generator:Call(point, Type, Block)
    end

    bcast(GenPoints.pre_enum)
    Block:DocString(Type.description)
    if Type.imported then
        Block:MakeAlias(Type:LocalName(), Type.location)
        bcast(GenPoints.enum_import)
    else
        if Type.type then
			Block:BeginBlockLine { "enum class ", Type:LocalName(), " : ", Type.type:LocalName(), " {", }
		else
			Block:BeginBlockLine { "enum class ", Type:LocalName(), " {", }
		end

        bcast(GenPoints.enum_entry)

        local nconf = Type.namespace.config

        if Type.values then
            for i,v in ipairs(Type.values) do
                Block:DocString(v.description)
                if v.value then
                    Block:Line { v.name, " = ", v.value, ",", }
                else
                    Block:Line { v.name, ",", }
                end
            end
        end

        bcast(GenPoints.enum_exit)
        Block:EndBlockLine { "};", }
    end
    bcast(GenPoints.post_enum)
end

function Enum:GenereateReset(Type, Block, Generator, variable, default)
	local value
	default = default or Type:GetDefault()

	if default == nil then
		Block:Line {
			"// for ",
			variable,
			" of type ",
			Type:GlobalName(),
			" default value is not set",
		}
        Block:Line { variable, " = static_cast<", Type:GlobalName(), ">(0);", }
	else
		Block:Line { variable, " = ", Type:GlobalName(), "::", default, ";", }
    end
end

return x2c.MakeGenerator(Enum)

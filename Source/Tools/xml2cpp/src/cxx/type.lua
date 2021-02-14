local GenPoints = require("observer").GenPoints

local TypeGen = { }

function TypeGen:GenereateReset(Type, Block, Generator, variable, default )--member, name, writter, exportsettings, default)
	local value
	default = default or Type:GetDefault()

	if Type.AssignCast then
		value = string.format(Type.AssignCast, tostring(default))
	else
		value = string.format("static_cast<%s>(%s)", Type:LocalName(), tostring(default))
	end

	Block:Line { variable, " = ", value, ";", }
end

return x2c.MakeGenerator(TypeGen)

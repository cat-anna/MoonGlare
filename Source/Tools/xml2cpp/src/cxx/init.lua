
require "cxx/exporters"

local Assert = x2c.Assert
local GenPoints = x2c.Observer.GenPoints

local Generators = { }

Generators.Structure = require "cxx/structure"
Generators.Type = require "cxx/type"
Generators.Enum = require "cxx/enum"
Generators.Alias = require "cxx/alias"

table.join(Generators, require "cxx/container")

----------------------------------------------------------------------------------------------------

return Generators

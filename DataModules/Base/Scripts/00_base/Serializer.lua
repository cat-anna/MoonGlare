--part of MoonGlare engine internal module

Utils = Utils or {}

local StringIO = {}
local StringIO_mt = { 
	__index = StringIO,
	__tostring = function(o)
		return o:tostring()
	end,
}

function StringIO:write(part, ...)
	if part == nil then
		return
	end
	self.parts[#self.parts + 1] = part
	return self:write(...)
end

function StringIO:tostring()
	local r = ""
	for i,v in ipairs(self.parts) do
		r = string.format("%s%s", r, tostring(v))
	end
	return r
end

local function StringIO_create()
    local new_inst = {
		parts = { },
	}   
    setmetatable( new_inst, StringIO_mt )
    return new_inst
end

local function _DoSerializePretty(data, output, pretty)
    local f = output
    local dump
    
    dump = function (value, loc)
        local t = type(value)
        local action = {
            table = function(value, loc)
                f:write("{")
				if pretty then 
					f:write("\n")
				end
                for v,k in pairs(value) do
                    if pretty then 
						for i = 1,(loc+1) do f:write("\t"); end
					end
					
					if type(v) == "string" then
						v = string.format("\"%s\"", v)
					else	
						v = tostring(v)
					end
					
                    f:write("[", v, "]")
					if pretty then 
						f:write(" = ")
					else
						f:write("=")
					end
                    dump(k, loc+1)
                end
                if pretty then 
					for i = 1,loc do f:write("\t"); end
				end
                f:write("}")
            end,
            string = function(value, loc)
                f:write("\"", value, "\"")
            end,
        }
        local nodef = action[type(value)]
		if nodef then
			nodef(value, loc)
		else
			 f:write(tostring(value))
		end
        if loc > 0 then
            f:write(",")
        end
		if pretty then 
			f:write("\n")
		end		
    end
	
	dump(data, 0)

end

function Utils.Serialize(data, pretty)
	local sio = StringIO_create()
	_DoSerializePretty(data, sio, pretty)
    return sio:tostring()
end

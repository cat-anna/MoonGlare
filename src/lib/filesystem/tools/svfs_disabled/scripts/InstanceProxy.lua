
helpers = helpers or { }

function helpers.CreateInstanceProxy(Inst)
	local mt = { 
		__index = function(table, key)
			local get = rawget(table, key)
			if get then 
				return get
			end
			
			if not inst or not inst[Inst] then
				return nil
			end
			
			local i = inst[Inst]
			if not i[key] then
				return nil
			end
			
			get = function(...)
				local i = inst[Inst]
				return i[key](i, ...)
			end

			rawset(table, key, get)
			return get
		end,
		__newindex = function()
			return nil
		end
		
	}
	
	local inst = { }
	setmetatable(inst, mt)
	return inst
end

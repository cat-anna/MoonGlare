--part of MoonGlare engine internal module
-- object-oriented support for lua
 
oo = { }
 
function oo.InheritsFrom( base )
    local class = { }
	
	local classmt = { 
		__index = class
	}
 
	function class:new()
		body = { }
		body.Class = class
		body.BaseClass = base
		setmetatable( body, classmt )
		return body
	end
         
    if base then
		local basemt = {
			__index = base,
		}
		setmetatable(class, basemt)
    end
        
	class.BaseClass = base
 
    return class
end
 
function oo.NewClass()
	return oo.InheritsFrom()
end

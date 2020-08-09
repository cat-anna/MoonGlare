
local org_print = print 

if not table.unpack then
    --compatibiity
    table.unpack = unpack
end

function print(...)
   org_print(...)
   io.flush()
end

utils = utils or { }

function utils.explode(div,str)
    if (div=='') then return false end
    local pos,arr = 0,{}
    for st,sp in function() return string.find(str,div,pos,true) end do
        table.insert(arr,string.sub(str,pos,st-1))
        pos = sp + 1
    end
    table.insert(arr,string.sub(str,pos))
    return arr
end

function SubPointerCallMetaMethod(table, key)
	local v = table.p
	if not v then
		print "SubPointerCallMetaMethod: v is nil!"
		os.exit(1)
	end
	local f = v[key]
	if not f then
		print "SubPointerCallMetaMethod: f is nil!"
		return nil
	end
	return function(t, ...)
		return f(v, ...)
	end
end



local function exportstring( s )
    return string.format("%q", s)
end

function table.serialize( tbl )
    local out = { }
    local function put(d)
        table.insert(out, d)
    end

    local dump
    dump = function (value, loc)
        local t = type(value)
        local action = {
            table = function(value, loc)
                put("{\n")
                for k,v in pairs(value) do
                    put(loc);
                    if type(k) == "string" then
                        put("[" .. exportstring(k) .. "] = ")
                    else
                        put("[" .. tostring(k) .. "] = ")
                    end
                    dump(v, loc .. "\t")
                end
                put(loc)
                put("}")
            end,
            number = function(value, loc) put(tostring(value)) end,
            string = function(value, loc) put(exportstring(value)) end,
            boolean = function(value, loc) put(tostring(value)) end,
        }
        local h = (action[type(value)])
        if h then 
            h(value, loc);
        else
            put("nil") 
            put(" -- ") 
            put(type(value)) 
        end
        if loc:len() > 0 then
            put(",\n")
        else
            put("\n")
        end
    end 
    dump(tbl, "")
    return table.concat(out, "")
 end
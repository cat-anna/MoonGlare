
local Observer = { }
x2c.Observer = Observer

---------------------------------------

local ProtoPoints = {
    "file_start",
    "file_end",

    "pre_struct_declartion",
    "post_struct_declartion",
    "pre_struct_definition",
    "post_struct_definition",
    "struct_import",
    "struct_entry",
    "struct_exit",
    "pre_struct_members",
    "post_struct_members",
    "post_struct",
    "struct_public",

--alias
    "pre_alias",
    "post_alias",

--enum
    "pre_enum",
    "enum_import",
    "enum_entry",
    "enum_exit",
    "post_enum",

--Container
    "pre_table",
    "post_table",
    "pre_list",
    "post_list",
    "pre_map",
    "post_map",
}

local function PreprocessGenPoints(points)
    local ret = { }
    for _,v in ipairs(points) do
        ret[v] = v
    end
    return ret;
end

local GenPoints = PreprocessGenPoints(ProtoPoints)
Observer.GenPoints = GenPoints

---------------------------------------

local ObserverBroadcast = {}
local ob_mt = {
    __index = function(object, name)
        local f = ObserverBroadcast[name]
        if f then
            return f
        end

        if GenPoints[name] then
            return function(self, ...)
                return ObserverBroadcast.Bcast(self, name, ...)
            end
        end

        error("There is no such observer action: ", name)
    end
 }
x2c.Classes.ObserverBroadcast = ObserverBroadcast

function ObserverBroadcast.New()
    local o = { }
    o.recivers = { }
    return setmetatable(o, ob_mt)
end

function ObserverBroadcast:Add(reciver)
    self.recivers[#self.recivers + 1] = reciver
end

function ObserverBroadcast:Bcast(call, ...)
    for _,v in ipairs(self.recivers) do
        local f = v[call]
            if f then
            if type(f) ~= "function" then
                error "Not a function"
            end
            f(v, ...)
        end
    end
end

return Observer

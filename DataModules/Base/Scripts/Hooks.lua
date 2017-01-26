
local Hooks = { }
_G["Hooks"] = Hooks

Hooks.Recivers = { }

function Hooks.RegisterReciver(reciver)
    if type(reciver) ~= "table" then
        error("HOOKS: RegisterReciver invalid argument")
        return
    end

    Hooks.Recivers[#Hooks.Recivers + 1] = reciver
end

function Hooks.Dispatch(event, arg)
    for _,v in ipairs(Hooks.Recivers) do
        local f = v[event]
        if type(f) == "function" then
            f(v, event, arg)
        end
    end
end

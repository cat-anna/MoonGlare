local ThreadList = {}
local SuspendedThreads = {}
local Threadidcnt = 0

--[[@ [LuaThreadModule/_] `coroutine.create(function[, name])`
    Create runnable coroutine. This is only a wrapper.
@]]
local org_coroutine_create = coroutine.create
function coroutine.create(f, name)
    Threadidcnt = Threadidcnt + 1
    local thread = {
        name = name,
        id = Threadidcnt,
    }

    thread.handle = org_coroutine_create(f)
    table.insert(ThreadList, thread)
    return thread.handle
end   

--[[@ [LuaThreadModule/_] `coroutine.sleep(seconds)`
    Suspend execution of current coroutine. 
@]]
function coroutine.sleep(sec)
    local h = coroutine.running()
    if h == nil then
        return        
    end

    for _,v in ipairs(ThreadList) do
       if v.handle == h then
           v.timeout = Time.global + sec
           table.insert(SuspendedThreads, v)
           table.sort(SuspendedThreads, function(a,b) return a.timeout < b.timeout end)
           coroutine.yield()           
           return
       end
    end
    --TODO: some error
end                

local function DoStep()

    local t = Time.global
    while #SuspendedThreads > 0 and SuspendedThreads[1].timeout < t do
        local t = table.remove(SuspendedThreads, 1)
        t.timeout = nil
        table.insert(ThreadList, t)
    end

    local cnt = math.min(5, #ThreadList)
    for i=1,cnt do
        local front = table.remove(ThreadList, 1)   
        if front.dirty or front.timeout then
            --nothing           
        else
            front.dirty = true
            table.insert(ThreadList, front)
            local succ, errm = coroutine.resume(front.handle)
            if not succ then
               Log.Error("Thread error: " .. errm)
            else
               front.dirty = coroutine.status(front.handle) == "dead"
            end
        end 
    end
end

return DoStep
           
local ThreadList = {}
local SuspendedThreads = {}
local LoaderThreadCnt = 0
local LoaderThreadChange = true
local Threadidcnt = 0

local org_coroutine_create = coroutine.create
function coroutine.create(f, name, isloader)
    Threadidcnt = Threadidcnt + 1
    local thread = {
        loader = isloader,
        name = name,
        id = Threadidcnt,
    }

    if isloader then
        LoaderThreadCnt = LoaderThreadCnt + 1
        LoaderThreadChange = true
    end
    
    thread.handle = org_coroutine_create(f)
    table.insert(ThreadList, thread)
    return thread.handle
end   

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

function DoStep()

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
            if front.dirty and front.loader then
                LoaderThreadCnt = LoaderThreadCnt - 1
                LoaderThreadChange = true
            end
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
    local prv = LoaderThreadChange
    LoaderThreadChange = false
    return prv, LoaderThreadCnt
end

return DoStep
           
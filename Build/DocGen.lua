
local lfs = require "lfs"

function string.trim(s)
    return s:match "^%s*(.-)%s*$"
end

function string.split(str, pat)
    local t = {}  -- NOTE: use {n = 0} in Lua-5.0
    local fpat = "(.-)" .. pat
    local last_end = 1
    local s, e, cap = str:find(fpat, 1)
    while s do
       if s ~= 1 or cap ~= "" then
          table.insert(t,cap)
       end
       last_end = e+1
       s, e, cap = str:find(fpat, last_end)
    end
    if last_end <= #str then
       cap = str:sub(last_end)
       table.insert(t, cap)
    end
    return t
end

function string.dup(str, n)
    local r = ""
    for i=1,n do 
        r = r .. str
    end
    return r
end

-------------------------------------------------

local DocGen = { }

DocGen.stat = {
    files=0,
    dirs=0, 
    entries=0,   
}

function DocGen.stat:IncFile() self.files = self.files + 1 end
function DocGen.stat:IncDir() self.dirs = self.dirs + 1 end
function DocGen.stat:IncEntry() self.entries = self.entries + 1 end

function DocGen.stat:Print()
    print(string.format("Processed dirs: %d", self.dirs))
    print(string.format("Processed files: %d", self.files))
    print(string.format("Processed entries: %d", self.entries))
end

-------------------------------------------------

local function GetFileData(fileName)
    local f = io.open(fileName, "r")
    local str = f:read("*a")
    f:close()
    return str
end

-------------------------------------------------

function DocGen:ProcessSourceFile(fileName)
    local ext = fileName:match("%.(%w+)$") or ""
    ext = ext:lower()
    local extMapping = self.fileExts[ext]
    if not extMapping then
        return true
    end
    local searchRegex = self.searchRegex[extMapping]
    if not searchRegex then
        return false, "Invalid search regex"
    end

    local fileData = GetFileData(fileName)

    local entries = self.entries

    for v in string.gmatch(fileData, searchRegex) do
        self.stat:IncEntry()

        local opts, header, text = v:match("%s*%[([^%[%]]+)%]%s*([^\n]+)%s+(.*)")
        -- print(opts .. "|" .. header:trim() .. "|" .. text:trim())

        opts = opts:trim():split(";")
        local topicPath = opts[1] --TODO
        local topic = topicPath:split("/")
        topic.relative = topicPath:sub(1,1) ~= "/"
        topic.path = topicPath  
        
        if topic[#topic] == "_" then
            table.remove(topic)
            topic.nameless = true
        end

        -- print(topic.relative, topic.path, topic.nameless, #topic, topic[1], topic[2], topic[3])

        table.insert( entries, {
            filePath = fileName,            
            opts = opts,
            topic = topic,
            header = header:trim(),
            rawText = text:trim(),
            children = {},
            entries = {},
        })
    end

    -- print("Processing file: " .. fileName)
    self.stat:IncFile()
    return true
end

function DocGen:ProcessDirectory(path)
    for file in lfs.dir(path) do
        if file ~= "." and file ~= ".." then
            local f = path..'/'..file
            local attr = lfs.attributes (f)
            if attr.mode == "file" or attr.mode == "directory" then 
                table.insert( self.src, { attr = attr, path = f } )
            end
        end
    end
    self.stat:IncDir()
    return true
end

function DocGen:ImportSources()
    print("Importing sources")
    while #self.src > 0 do
        local v = table.remove(self.src, 1)
        if v.attr.mode == "file" then
            if not self:ProcessSourceFile(v.path) then
                return false
            end
        elseif v.attr.mode == "directory" then
            if not self:ProcessDirectory(v.path) then
                return false
            end
        end
    end
    return true
end

function DocGen:FindOrAllocateTopic(entry)
    local topic = entry.topic
    if #topic < 1 then
        return nil, "Invalid topic format"
    end
    local last = topic[#topic]
    local cached = self.topicsCache[last]

    if cached then 
        if topic.nameless then
            table.insert(cached.entries, entry)
            return entry
        end
    
        return cached
    end

    if topic.relative and #topic == 1 then
        return nil, "Cannot find parent topic: " .. last
    end

    local lastParent
    for n,v in ipairs(topic) do
        local currentParent = self.topicsCache[v]
        if not currentParent then
            if n ~= #topic and topic.relative then
                return nil, "Cannot find parent topic: " .. last
            end

            local node = entry

            node.parent = lastParent
            node.children = node.children or { }
            node.entries = node.entries or { }
            self.topicsCache[v] = node
            -- print("ADD CACHE: ", v, lastParent)
            if lastParent ~= nil then
                table.insert(lastParent.children, node)
            else
                table.insert(self.topicsTree, node)
            end
            lastParent = node
        else
            lastParent = currentParent
        end            
    end

    return entry
end

function DocGen:ProcessTopic(topic, entry)
    local succ, error = self:FindOrAllocateTopic(entry)
    if not succ then
        entry.lastError = error
        return false
    end

    return true
end

function DocGen:ProcessEntry(entry)
    if not self:ProcessTopic(entry.topic, entry) then
        return false
    end

    --TODO

    return true
end

function DocGen:ProcessEntries()
    print("Processing entries")
    local entries = { }
    for _,v in ipairs(self.entries) do
        table.insert(entries, v)
    end

    local sthProcessed = true 
    while sthProcessed and #entries > 0 do 
        sthProcessed = false
        local processedCnt = 0
        local delayedEntries = { }

        while #entries > 0 do
            local entry = table.remove(entries, 1)
            if self:ProcessEntry(entry) then
                sthProcessed = true                     
                processedCnt = processedCnt + 1
            else
                table.insert(delayedEntries, entry)
            end
        end

        entries = delayedEntries
        print("Processed Entries in loop: " .. tostring((processedCnt)))
    end

    if #entries > 0 then
        print("Error: not all entries were processed: " .. tostring(#entries))
    end
    return true
end


function DocGen:OpenOutput()
    local f = io.open("bin/doc.md", "w")

    local mt = { } 
    mt.__index = mt

    function mt:Close() self.f:close() self.f = nil end

    function mt:Write(str) if str then self.f:write(str) end end
    function mt:Line(str) self:Write(str .. "\n") end

    function mt:Header(str, lvl) if str then self:Line(string.dup("#", lvl) .. " " .. str) end end
    function mt:Footer(str, lvl) self:Line("") end

    function mt:List(str) if str then self:Write("* **" .. str .. "**  \n") end end

    return setmetatable({ f = f }, mt)
end

function DocGen:GenerateEntryOutput(out, entry, level)
    out:Header(entry.header, level)
    out:Line(entry.rawText)

    for _,v in ipairs(entry.entries) do
        out:List(v.header)
        out:Line(v.rawText)
        out:Line("")
    end

    for _,v in ipairs(entry.children) do
        self:GenerateEntryOutput(out, v, level + 1)
    end

    out:Footer("", level)
end

function DocGen:GenerateOutput()
    local output, errm = self:OpenOutput()
    if not output then
        return false, errm
    end

    output:Write[[
Auto-generated documentation. Do not edit directly.

]]

    for _,v in ipairs(self.topicsTree) do
        self:GenerateEntryOutput(output, v, 1)
    end

    output:Close()
    return true
end

function DocGen:Generate()
    self.entries = {}
    self.topicsCache = { }
    self.topicsTree = { }

    if not self:ImportSources() then
        return false, "Importing sources failed"
    end
    if not self:ProcessEntries() then
        return false, "Processing entries failed"
    end
    if not self:GenerateOutput() then
        return false, "Generating output failed"
    end

    return true
end

function DocGen:ProcessArguments(argTable)
    print("Processing arguments")
    self.src = { }
    self.output = ""

    for _,v in ipairs(argTable) do
        local attr = lfs.attributes(v)
        if attr then
            if attr.mode == "file" or attr.mode == "directory" then 
                table.insert( self.src, { attr = attr, path = v } )
            end
        else
            local arg, value = v:match("%-%-(%w+)=?(%w*)")
            if not arg then
                print("Ignoring invalid argument: " .. v)
                -- TODO
            elseif arg == "output" then
                    self.output = value
                    self.output = self.output .. "/"
            else
                print("Ignoring unknown argument: " .. arg)
            end
        end
    end
    return true
end

function DocGen:RunApp(argTable)

    if not self:ProcessArguments(argTable) then
        return 
    end
    local succ, errm = self:Generate()
    if not succ then
        print("ERROR:" .. errm)
        return
    end
    self.stat:Print()
end

DocGen.fileExts = { 
    c="cxx", 
    cpp="cxx", 
    h="cxx", 
    hpp="cxx", 
    lua="lua" ,
}

DocGen.searchRegex = {
    cxx = "/%*@(.-)@%*/",
    lua = "%-%-%[%[@(.-)@%]%]",
}

return setmetatable({}, {__index=DocGen}):RunApp(arg)

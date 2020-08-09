
vfs = inst.svfs
Register = vfs:GetRegister()

local Path = { }

function prompt()
	local p = ConcatPath(Path)
	return p .. "> "
end

local function SplitPath(subpath)
	local p = { }

	local r = utils.explode("/", subpath)
	
	if not r[1] or r[1] == "" then
	else
		for i,v in ipairs(Path) do
			p[#p + 1] = v
		end	
	end
	
	for i,v in ipairs(r) do
	--	print(i, v)
		if v == ".." then
			table.remove(p, #p)
		elseif v == "." or not v or v == "" then
			--nop
		else
			p[#p + 1] = v
		end
	end
	
	return p
end

function ConcatPath(p)
	return "/" .. table.concat(p, "/") 
end

Help.Register { Command="path", Brief="get full path of argumet", Usage="path(ELEMENT)" }
function path(subpath)
	if not subpath or subpath == "" then
		return pwd()
	end
	
	return ConcatPath(SplitPath(subpath))
end

Help.Register { Command="fileinfo", Brief="get file info from id", Usage="fileinfo(FileID)" }
function fileinfo(id)
	id = tonumber(id)
	if not id then
		print "fileinfo requires id of file!"
		return nil
	end

	if not vfs:IsFileValid(id) then
		return nil
	end
	
	local fname = vfs:GetFileName(id)
	local fsize = vfs:GetFileSize(id)

	return {
		id = function () return id end,
		name = function() return fname end,
		size = function() return fsize end,
		fullpath = function() return vfs:GetFullFilePath(id) end,
		directory = function() return vfs:IsFileDirectory(id) end,
	}
end

Help.Register { Command="cd", Brief="goto folder", Usage="cd(FOLDER)" }
function cd(subpath)
	subpath = subpath or "/"

	local r = SplitPath(subpath)
	local rp = ConcatPath(r)
	
	local h = vfs:OpenFile(rp, 1, 1)
	if h:IsValid() == 0 then
		print "Not a valid file"
		return 
	end
	
	if h:IsDirectory() == 0 then
		print "Cannot enter into file!"
		return
	end
	
	Path = r
	stdout(rp)
end

Help.Register { Command="pwd", Brief="get current directory" }
function pwd()
	return ConcatPath(Path)
end

Help.Register { Command="ls", Brief="list content of directory", Usage="ls([PATH])" }
function ls(p)
	if p then
		p = path(p)
	else
		p = pwd()
	end
	
	local h = vfs:OpenFile(p, 1, 1)
	if h:IsValid() == 0 then
		return
	end
	
	local fidt = h:GetChildren()
	local files = { }
	for i,v in ipairs(fidt) do
		files[#files + 1] = fileinfo(v)
	end
	
	table.sort(files, function(a, b)
		if not a.lname then
			a.lname = string.lower(a.name())
		end
		if not b.lname then
			b.lname = string.lower(b.name())
		end		
		return a.lname < b.lname
	end)
	
	local sfmt = function(s)
		if s < 1000 then
			return tostring(s) .. "B"
		end
		local t = { "B", "K", "M", "G" }
		local i = 1
		while (s > 1000) do
			s = s / 1024.0
			i = i + 1
		end
		return string.format("%5.1f%s", s, t[i])
	end
	
	for i,v in ipairs(files) do
		local flags = { "" }
		if v.directory() > 0 then
			flags[#flags + 1] = "D"
		end
		stdout(string.format("%6d. %4s %8s %s", v.id(), table.concat(flags), sfmt(v.size()), v.name()))
	end
end

Help.Register { Command="mount", Brief="mount container", Usage="mount(CONTAINER, [MOUNTPOINT])" }
function mount(what, where)
	if not what then
		return
	end
	where = where or "/"
	
	return vfs:OpenContainer(what, where, 0)
end

Help.Register { Command="lsmod", Brief="TBD" }
function lsmod()
end

Help.Register { Command="addmod", Brief="TBD" }
function addmod(name, args)
end

Help.Register { Command="mkdir", Brief="create directories", Usage="mkdir(PATH)"}
function mkdir(p)
	if not p then
		print "mkdir requires an argument"
		return 1
	end
	p = path(p)
	
	return vfs:ForcePath(p)
end

Help.Register { Command="read", Brief="read file content, returns nil on failure, on succes string then length", Usage="read(filename)"}
function read(filename)
    if not filename then
        return nil
    end
    filename = path(filename)
	local h = vfs:OpenFile(filename, 1, 1)
	if h:IsValid() == 0 then
		return nil
	end

    local data
    local len
    data, len  = h:GetFileData()
    if not data then
        return data
    end
    return data, len
end

Help.Register { Command="rm", Brief="remove file", Usage="rm(FILE)"}
function rm(path)
    assert(type(path) == "string")            
    local fid = vfs:FindFile(path)
    if fid > 0 then
        return vfs:DeleteFile(fid)
    end

	local h = vfs:OpenFile(pwd(), 1, 1)
	if h:IsValid() == 0 then
		return false
	end
	
	local fidt = h:GetChildren()
	for i,v in ipairs(fidt) do
		local fi = fileinfo(v)
        local fp = fi.fullpath()
        if fp:match(path) then
            vfs:DeleteFile(vfs:FindFile(fp))
        end
	end
    return true
 end

 Help.Register { Command="injectFile", Brief="Inject file into vfs", Usage="injectFile(VFS_PATH, SYSTEM_PATH)" }
 local injectContainer
function injectFile(virtual, system)
    if type(virtual) ~= "string" then
        print("invalid VFS_PATH")
        return nil
    end
    if type(system) ~= "string" then
        print("invalid SYSTEM_PATH")
        return nil
    end
    if not injectContainer then
        injectContainer = vfs:CreateContainer("VirtualFileContainer")
    end
    return injectContainer:InjectFile(system, virtual, true)
end
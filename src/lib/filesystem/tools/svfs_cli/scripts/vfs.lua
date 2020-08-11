
local current_path = { }

local function ConcatPath(p)
	p = p or {}
	return (#p > 0 and "/" or "") .. table.concat(p, "/") 
end

local function SplitPath(subpath)
	local p = { }

	local r = utils.explode("/", subpath)
	
	if not r[1] or r[1] == "" then
	else
		for i,v in ipairs(r) do
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

function prompt()
	local p = ConcatPath(current_path)
	return p .. "> "
end

Help.Register { Command="absolute", Brief="get absolute path of element relative to cwd", Usage="absolute(ELEMENT)" }
function absolute(relative)
	local subpath = relative
	if not subpath or subpath == "" then
		return pwd()
	end

	if subpath:sub(1,1) ~= "/" then
		subpath = pwd() .. "/" .. subpath
	end
	
	return ConcatPath(SplitPath(subpath))
end

--[[
vfs = inst.svfs
Register = vfs:GetRegister()

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
]]

Help.Register { Command="pwd", Brief="get current directory" }
function pwd()
	return ConcatPath(current_path)
end

Help.Register { Command="ls", Brief="Print content of directory", Usage="ls([PATH])" }
function ls(p)
	if p then
		p = absolute(p)
	else
		p = pwd()
	end

	local r = StarVfs:EnumeratePath(p);
	for i=1, #r do
		print((r[i].is_directory and "d" or " ") .. "   " ..  r[i].file_name)
	end
end

Help.Register { Command="mount_host_folder", Brief="mount host folder", Usage="mount_host_foder(host_path, [monunt_point])" }
function mount_host_folder(host_path, monunt_point, opt_args)
	-- if not host_path  then
	-- 	return
	-- end
	opt_args = opt_args or {}
	opt_args.host_path=host_path
	opt_args.monunt_point=absolute(monunt_point)
	return StarVfs:MountContainer("host_folder", opt_args)
end

--[[
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
end]]

Help.Register { Command="read", Brief="read file content, returns nil on failure, on succes string then length", Usage="read(filename)"}
function read(filename)
    if not filename then
        return nil
    end
	return StarVfs:ReadFileByPath(absolute(filename))
end

--[[
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

]]


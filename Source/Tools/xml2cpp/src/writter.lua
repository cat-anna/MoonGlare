
local FileBlock = inheritsFrom(nil)

function FileBlock:Init(Writter)
	self.Writter = Writter
	self.Block = 0
	self.Enabled = true
	self.Lines = { }
	self.NamedBlocks = { }
end

function FileBlock:GetWritter()
    return self.Writter
end

function FileBlock:Ident()
    return string.rep(self.Writter.Ident, self.Block)
end

function FileBlock:SetLinePrefix(LinePrefix)
	self.LinePrefix = LinePrefix
end

function FileBlock:BeginBlock()
	self.Block = self.Block + 1
end

function FileBlock:EndBlock()
	self.Block = self.Block - 1
	if self.Block < 0 then
		self.Block = 0
	end
end

function FileBlock:BlockLine(parts)
	self:BeginBlock()
	self:Line(parts)
	self:EndBlock()
end

function FileBlock:BeginBlockLine(parts)
	self:Line(parts)
	self:BeginBlock()
end

function FileBlock:EndBlockLine(parts)
	self:EndBlock()
	self:Line(parts)
end

function FileBlock:BlockFormat(...)
	self:BeginBlock()
	self:Format(...)
	self:EndBlock()
end

function FileBlock:AddChildBlock(name)
	local b = self.Writter:CreateBlock()
	b.Block = self.Block
	if name then
		self.NamedBlocks[name] = b
	end

	self.Lines[#self.Lines + 1] = b
	return b
end

function FileBlock:FindBlock(name)
	return self.NamedBlocks[name]
end

function FileBlock:Format(str, args)
	if type(str) == "table" then
		str = table.concat(str, "")
	end

	for k,v in pairs(args or { } ) do
		local key = string.format("{%s}", k)
		if str:find(key) then
			local value = v
			if type(v) == "function" then
				value = v(args)
			end
			str = str:gsub(key, value)
		end
	end
	return self:Line(str)
end

function FileBlock:Line(parts)
	if not self.Enabled then
		return
	end

	local line = { }
	self.Lines[#self.Lines + 1] = line

	local put = function(v)
		if not v then
			v = "[nil]"
		end
		line[#line + 1] = v
	end

	put(self:Ident())
	if self.LinePrefix then
		put(self.LinePrefix)
	end

	if type(parts) == "table" then
		for i,v in ipairs(parts) do
			put(v)
		end
	elseif type(parts) == "string" then
		put(parts)
	end
end

function FileBlock:Write(f)
	for _,v in ipairs(self.Lines) do
		if v.Write then
			v:Write(f)
		else
			local line = table.concat(v, "")
			f:write(line)
		end
	end
	f:write("")
end

---------------------------------------

local Writter = {}

x2c.Classes.Writter = Writter

function Writter:Init(FileName)
	self.Ident = "\t"
	self.FileBlocks = { }
	self.NamedBlocks = { }

	self.ImplLines = { }
	self.DefLines = { }
	self.enabled = true
    self.FileName = FileName
end

function Writter:CreateBlock()
	local b = FileBlock:Create(self)
	if self.BlockExt then
		for k,v in pairs(self.BlockExt) do
			b[k] = v
		end
	end
	return b
end

function Writter:AddFileBlock(name)
	local b = self:CreateBlock()

	self.FileBlocks[#self.FileBlocks + 1] = b
	if name then
		self.NamedBlocks[name] = b
	end
	return b
end

function Writter:FindFileBlock(name)
	return self.NamedBlocks[name]
end

---------------------------------------

function Writter:WriteFileHeader()
	error("Not implemented")
end

---------------------------------------

function Writter:Close()
	local f = io.open(self.FileName, "w")
	if not f then
		error("Unable to open file " .. filename .. " for writting")
	end

	local fproxy = {
		write = function(self, line)
			local s = trim(line)
			local empty = s:len() == 0
			if not empty then
				f:write(line)
				f:write("\n")
				self.line = true
			else
				if self.line then
					f:write("\n")
				end
				self.line = false
			end
		end,
	}

	for i,block in ipairs(self.FileBlocks) do
		block:Write(setmetatable({}, { __index = fproxy, }))
	end
	f:close()
	return true
end

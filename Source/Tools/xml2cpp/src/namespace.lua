
local Namespace, Namespace_mt = x2c.MakeTypeClass()

function Namespace.new(name, parent)
    local new_inst = {}

	new_inst.Internals = { }
	new_inst.config = { }
	new_inst.parent = parent
	new_inst.name = name
	new_inst.object_type = "Namespace"

	if parent then
		new_inst.config = table.shallow_clone(parent.config);
	end

    setmetatable( new_inst, Namespace_mt )

	if parent then
		parent:Add(new_inst)
	end

    return new_inst
end

function Namespace:Add(t)
	local i = self.Internals[t:GetName()]
	if i then
		error("Cannot define ", t:GetName(), " in namespace ", i:GlobalName(), " - already exists")
		return nil
	end

	self.Internals[t:GetName()] = t
end

function Namespace:Get(name)
	local i = self.Internals[name]
	if not i then
		if self.parent then
			return self.parent:Get(name)
		end

		return nil
	end

	return i
end

function Namespace:Exists(name)
	local i = self.Internals[name]
	if not i then
		if self.parent then
			return self.parent:Get(name)
		end

		return false
	end

	return true
end

function Namespace:GlobalName()
	if not self.parent then
		return ""
	end

	return self.parent:GlobalName() .. "::" .. self.name
end

function Namespace:DisplayName()
	if not self.parent then
		return "::"
	end
	return self:GlobalName()
end

function Namespace:WriteEnter(writter)
    local n = { }

    local it = self
    while it.parent do
        table.insert(n, 1, it:LocalName())
        it = it.parent
    end

	writter:Line {
		"namespace ",
		table.concat(n, "::"),
		" {",
	}
	writter:BeginBlock()
end

function Namespace:WriteLeave(writter)
	writter:EndBlock()
	writter:Line {
		"}",
	}
end

------------------------------------------

local NamespaceIF = { }
local NamespaceIF_mt = { }

function NamespaceIF_mt.__tostring(self)
	return tostring(rawget(self, "Namespace"))
end

function NamespaceIF_mt.__index(self, key)

	if key == "Type" then
		return function()
			return rawget(self, "Namespace"):Type()
		end
	end

	local t = rawget(self, "Namespace"):Get(key)

	if not t then
		error(self, " There is no ", key)
	end

	return NamespaceIF.cover(t)
end

function NamespaceIF.cover(T)
	if T:Type() == "Namespace" then
		local IF = { }
		IF.Namespace = T
		setmetatable(IF, NamespaceIF_mt)
		return IF
	else
		return T
	end
end


------------------------------------------

namespace = { }
local namespace_mt = { }
setmetatable(namespace, namespace_mt)

function namespace_mt.__call(n, NName, ReturnOnly)
	local parts = NName:split(".")

	local current = x2c.GlobalNamespace

	for i,v in ipairs(parts) do
		local n = current:Get(v)
		if not n then
			info("Creating namespace ", v, " in ", current:DisplayName())

			n = Namespace.new(v, current)
			current = n
		else
			if n:Type() ~= "Namespace" then
				error(n:GlobalName(), " is not a namespace!")
				return nil
			end

			current = n
		end
	end

	if not ReturnOnly then
--		x2c.CurrentNamespace:Leave(x2c.output)
		x2c.CurrentNamespace = current
--		x2c.CurrentNamespace:Enter(x2c.output)
	end

	return current
end

------------------------------------------

Global = { }
local Global_mt = { __Index = Global }
setmetatable(Global, Global_mt)

function Global_mt.__index(self, name)
	local t = x2c.GlobalNamespace:Get(name)
	if not t then
		error("There is no ", name, " in global namespace")
		return nil
	end
	return NamespaceIF.cover(t)
end

function Global_mt.__newindex(self, name)
	error("Attempt to modify global context directly!")
end


------------------------------------------

Local = { }
local Local_mt = { __Index = Local }
setmetatable(Local, Local_mt)

function Local_mt.__index(self, name)
	local t = x2c.CurrentNamespace:Get(name)
	if not t then
		error("There is no ", name, " in current namespace")
		return nil
	end
	return NamespaceIF.cover(t)
end

function Local_mt.__newindex(self, name)
	error("Attempt to modify Local context directly!")
end

------------------------------------------

local GlobalMt = { }
local G = _G
function GlobalMt.__index(Gt, name)
	local v = rawget(G, name)
	if v then
		return v
	end
	return Local[name]
end

setmetatable(G, GlobalMt)

------------------------------------------

x2c.GlobalNamespace = Namespace.new()
x2c.CurrentNamespace = x2c.GlobalNamespace

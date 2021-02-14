
local utils = { }
x2c.utils = utils

if not table.unpack then
	table.unpack = unpack
end

function string:split(sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        self:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
end

function string:trim()
  return (self:gsub("^%s*(.-)%s*$", "%1"))
end

function table.shallow_clone(input)
	local t = { }
	for i,v in ipairs(input) do
		t[i] = v
	end
	for k,v in pairs(input) do
		t[k] = v
	end
	return t
end

function table.join(a, b)
	for k,v in pairs(b) do
		a[k] = v
	end
	return a
end

local function table_print (tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    local sb = {}
    for key, value in pairs (tt) do
      table.insert(sb, string.rep (" ", indent)) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        table.insert(sb, "{\n");
        table.insert(sb, table_print (value, indent + 2, done))
        table.insert(sb, string.rep (" ", indent)) -- indent it
        table.insert(sb, "}\n");
      elseif "number" == type(key) then
        table.insert(sb, string.format("\"%s\"\n", tostring(value)))
      else
        table.insert(sb, string.format(
            "%s = \"%s\"\n", tostring (key), tostring(value)))
       end
    end
    return table.concat(sb)
  else
    return tt .. "\n"
  end
end

function table.tostring( tbl )
    if  "nil"       == type( tbl ) then
        return tostring(nil)
    elseif  "table" == type( tbl ) then
        return table_print(tbl)
    elseif  "string" == type( tbl ) then
        return tbl
    else
        return tostring(tbl)
    end
end

local function xprint(line, sender, ...)
	local args

	if type(sender) == "table" then
		args = { sender, ": ", ... }
	else
		args = { sender, ... }
	end

	for i,v in ipairs(args) do
		line[#line + 1] = tostring(v)
	end
	print(table.concat(line, ""))
end

function error(...)
	print ""
	local line = {
		"ERROR: "
	}

	xprint(line, ...)
	print(debug.traceback())
	print ""
	os.exit(1)
end

function info(...)
	local line = {
		"INFO: "
	}
	xprint(line, ...)
end

function IfThen(cond, TrueVal, FalseVal)
	if cond then
		return TrueVal
	else
		return FalseVal
	end
end

function quote(str, char)
	if type(str) == "table" then
		str = table.concat(str, "")
	end
	char = char or "\""
	return string.format([[%s%s%s]], char, str, char)
end

function trim(s)
	  return s:match "^%s*(.-)%s*$"
end

-----------------------

local Assert = { }
x2c.Assert = Assert

function Assert.String(t, sender, msg, ...)
	if type(t) ~= "string" then
		error(sender, msg or "Invalid string provided ", ...)
	end
end

function Assert.Table(t, sender, msg, ...)
	if type(t) ~= "table" then
		error(sender, msg or "Invalid string provided ", ...)
	end
end

function Assert.type(t, sender, msg, ...)
	if not t or not t.Type then
		error(sender, msg or "Invalid type provided ", ...)
	end
end

function Assert.type_nonnamespace(t, sender, msg, ...)
	if not t or not t.Type or t:Type() == "Namespace" then
		error(sender, msg or "Invalid type provided ", ...)
	end
end

-----------------------
-- part of lua-backup project
-- object-oriented support for lua

function inheritsFrom( base, class )
    class = class or { }
    local mt = { __index = class }
    class.Base = base

    function class:Create(...)
       local inst = setmetatable( { } , mt )
       inst:Init(...)
       return inst
    end

    if base then
        setmetatable( class, { __index = base } )
    end

    return class
end

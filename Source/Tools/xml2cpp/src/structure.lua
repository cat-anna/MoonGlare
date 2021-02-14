
local Assert = x2c.Assert

-----------------------------------------------------------

local function make_structure(data)
    x2c.Exporter:InitTypeExporterInfo(data, "Structure")

	if data.fields then
		for i,v in ipairs(data.fields) do
			Assert.type_nonnamespace(v.type, Structure, "Structure member cannot be of type " , v.type)

            local member = string.format("%s%s%s",
                data.config.structure_field_prefix or "",
                v.name,
                data.config.structure_field_postfix or ""
            )
            v.decoratedname = member

            x2c.Exporter:InitTypeExporterMemberInfo(v, "Structure")

            setmetatable(v.exportsettings, {
                __index = function (self, value)
                    local r = rawget(self, value)
                    if r then
                        return r
                    end
                    return data.exportsettings[value]
                end,
                __newindex = function(self, name, value)
                    error(Structure, "Attempt to modify structure member ", v.name)
                end
            } )
		end
	end
    data.object_type = "Structure"
	local s = x2c.Exporter:MakeStructure(data)
	x2c.RegisterType(s, x2c.CurrentNamespace)
	return s
end

-----------------------------------------------------------

local StructureMeta = { }

function StructureMeta.new(data)
	if not data.name then
		error("Cannot define nameless structure")
		return false
	end

	if x2c.CurrentNamespace:Exists(data.name) then
		error(StructureMeta, "Attempt to redefine ", x2c.CurrentNamespace:Get(data.name))
	end

	data.imported = false
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config)

	data.classname = string.format("%s%s%s",
		data.config.structure_prefix or "",
		data.name,
		data.config.structure_postfix or ""
	)

	info("Defined structure ", data.classname, " in namespace ", data.namespace:DisplayName())

	make_structure(data)
end

function StructureMeta.import(data)
	if not data.name then
		error("Cannot define nameless structure")
		return false
	end
	if not data.location then
		error("Location must be specified while importing structure")
		return false
	end

	if x2c.CurrentNamespace:Exists(data.name) then
		error(StructureMeta, "Attempt to redefine ", x2c.CurrentNamespace:Get(data.name))
	end

	data.imported = true
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config)

	data.classname = string.format("%s%s%s",
		data.config.structure_prefix or "",
		data.name,
		data.config.structure_postfix or ""
	)
	data.decoratedname = data.classname

	info("Imported structure ", data.classname, " in namespace ", data.namespace:DisplayName(), " from ", data.location)

	make_structure(data)
end

function StructureMeta.prefix(value)
	Assert.String(value)
	x2c.CurrentNamespace.config.structure_prefix = value
end

function StructureMeta.postfix(value)
	Assert.String(value)
	x2c.CurrentNamespace.config.structure_postfix = value
end

-----------------------------------------------------------

local StructureFieldMeta = { }

function StructureFieldMeta.prefix(value)
	Assert.String(value)
	x2c.CurrentNamespace.config.structure_field_prefix = value
end

function StructureFieldMeta.postfix(value)
	Assert.String(value)
	x2c.CurrentNamespace.config.structure_field_postfix = value
end

-----------------------------------------------------------

x2c.MakeMetaSubObject(StructureMeta, StructureFieldMeta, "field")
x2c.MakeMetaObject(StructureMeta, "Structure")

local Assert = x2c.Assert
---------------------------------------

local function make_enum(data)
    x2c.Exporter:InitTypeExporterInfo(data, "Enum")
	
	if data.type and data.type:Type() == "Type" and not data.type.integral then
		error("Enums accept only internal integral types as type")
	end
	
	if data.values then
		local values = data.values
		data.values = { }
		for i,v in ipairs(values) do 
			local value
			if type(v) == "string" then
				value = {
					name = v,
				}
			else
				value = v
				Assert.Table(v, "Enum")
				Assert.String(v.name, "Enum", "Found nameless enum value!")
			end
					
			x2c.Exporter:InitTypeExporterMemberInfo(value, "Enum")
		
			local decorated = string.format("%s%s%s", 
				data.config.enum_value_prefix or "", 
				value.name,
				data.config.enum_value_postfix or ""
			)
			value.decorated = decorated
			data.values[#data.values + 1] = value
		end	
		
		if #data.values < 1 then
			data.values = nil
		end
	end
	
    data.object_type = "Enum"
	local e = x2c.Exporter:MakeEnum(data)
	x2c.RegisterType(e, x2c.CurrentNamespace)
	return e
end

---------------------------------------

local EnumMeta = { }

function EnumMeta.new(data)
	if not data.name then
		error("Cannot define nameless enum")
		return false
	end
	
	if x2c.CurrentNamespace:Exists(data.name) then
		return x2c.CurrentNamespace:Get(data.name) 
	end		
	
	data.imported = false
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);
	data.object_type = "Enum"
	
	data.local_name = string.format("%s%s%s", 
		data.config.enum_prefix or "", 
		data.name,
		data.config.enum_postfix or ""
	)
	
	if data.default then
		data.default_value = string.format("%s%s%s", 
			data.config.enum_value_prefix or "", 
			data.default,
			data.config.enum_value_postfix or ""
		)
	end
	
	local e = make_enum(data)	
	info("Defined enum ", e:LocalName(), " in namespace ", e.namespace:DisplayName())
end

function EnumMeta.import(data)
	if not data.name then
		error("Cannot define nameless enum")
		return false
	end
	
	if not data.location then
		error("Location must be specified while importing enum")
		return false
	end	
	
		
	if x2c.CurrentNamespace:Exists(data.name) then
		return x2c.CurrentNamespace:Get(data.name) 
	end		
	
	data.imported = true
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);
	
	data.enum_name = string.format("%s%s%s", 
		data.config.enum_prefix or "", 
		data.name,
		data.config.enum_postfix or ""
	)
	
	data.default_value = data.default
	
	info("Imported enum ", data.enum_name, " in namespace ", data.namespace:DisplayName(), " from ", data.location)
	local e = make_enum(data)
end

function EnumMeta.prefix(value)
	if type(value) ~= "string" then
		error("Invalid enum prefix value")
	end
	x2c.CurrentNamespace.config.enum_prefix = value
end

function EnumMeta.postfix(value)
	if type(value) ~= "string" then
		error("Invalid enum postfix value")
	end
	x2c.CurrentNamespace.config.enum_postfix = value
end

---------------------------------------

local EnumMetaValue = { }

function EnumMetaValue.prefix(value)
	if type(value) ~= "string" then
		error("Invalid enum value prefix value")
	end
	x2c.CurrentNamespace.config.enum_value_prefix = value
end

function EnumMetaValue.postfix(value)
	if type(value) ~= "string" then
		error("Invalid enum value postfix value")
	end
	x2c.CurrentNamespace.config.enum_value_postfix = value
end

---------------------------------------

x2c.MakeMetaSubObject(EnumMeta, EnumMetaValue, "value")
x2c.MakeMetaObject(EnumMeta, "Enum")


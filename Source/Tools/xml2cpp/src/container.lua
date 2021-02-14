
local Assert = x2c.Assert

-------------------------------

local ContainerType, ContainerType_mt = x2c.MakeTypeClass()

function ContainerType:GenResetToDefault(member, name, writter)
	writter:DefLine {
		member,
		".clear();",
	}
end

local function make_container(data)
	data.object_type = "Container"
	data.Generator = data.ContainerType
	local s = x2c.Exporter:MakeContainer(data)
	x2c.RegisterType(s, x2c.CurrentNamespace)
	return data
end

-------------------------------

ContainerMeta = { }

function ContainerMeta.table(ElementType, TypeName)
	Assert.type_nonnamespace(ElementType, ContainerMeta, " Table element cannot be of type ", ElementType)

	local name = TypeName or ElementType:LocalName() .. "Vector"
	if x2c.CurrentNamespace:Exists(name) then
		return x2c.CurrentNamespace:Get(name)
	end

	local data = { }
	data.ContainerType = "Table"
	data.element_type = ElementType
	data.name = name
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);

	data = make_container(data)
--	Write_Table(data, x2c.output)
	info("Defined table ", data:LocalName(), " in namespace ", data.namespace:DisplayName())
	return data
end

--[[
function Container.list(ElementType, TypeName)
	Assert.type_nonnamespace(ElementType, Container, " List element cannot be of type ", ElementType)

	local name = TypeName or ElementType:LocalName() .. "List"
	if x2c.CurrentNamespace:Exists(name) then
		return x2c.CurrentNamespace:Get(name)
	end

	local data = { }
	data.element_type = ElementType
	data.name = name
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);

	data = make_container(data)
	Write_List(data, x2c.output)
	info("Defined list ", data:LocalName(), " in namespace ", data.namespace:DisplayName())
	x2c.CurrentNamespace:Add(data)
	return data
end

function Container.map(KeyType, ElementType, TypeName)
	Assert.type_nonnamespace(KeyType, Container, " Map key cannot be of type ", KeyType)
	Assert.type_nonnamespace(ElementType, Container, " Map value cannot be of type ", ElementType)

	local name = TypeName or KeyType:LocalName() .. ElementType:LocalName() .. "Map"
	if x2c.CurrentNamespace:Exists(name) then
		return x2c.CurrentNamespace:Get(name)
	end

	local data = { }
	data.key_type = KeyType
	data.element_type = ElementType
	data.name = name
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);

	data = make_container(data)
	Write_Map(data, x2c.output)
	info("Defined map ", data:LocalName(), " in namespace ", data.namespace:DisplayName())
	x2c.CurrentNamespace:Add(data)
	return data
end
]]
x2c.MakeMetaObject(ContainerMeta, "Container")

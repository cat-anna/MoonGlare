
local Assert = x2c.Assert

-----------------------

local function AliasNamespace(self, Type, NewName)
	error(self, "Namespace aliasing is not supported")
end

local function AliasType(self, Type, NewName)
	data = { }

	data.source_type = Type
	data.name = NewName
	data.local_name = NewName
	data.Generator = "Alias"
	data.object_type = Type:Type()
	data.namespace = x2c.CurrentNamespace
	data.config = table.shallow_clone(data.namespace.config);

	local a = x2c.Exporter:MakeAlias(data)
	info("Aliased ", a, " to ", a.source_type)
	x2c.RegisterType(a, x2c.CurrentNamespace)
end

local AliasMeta = { }

function AliasMeta.__call(self, Type, NewName)
	Assert.type(Type, self, " Invalid type for aliasing: ", Type)

	if Type:Type() == "Namespace" then
		AliasNamespace(self, Type, NewName)
	else
		AliasType(self, Type, NewName)
	end
end

x2c.MakeMetaObject(AliasMeta, "Alias")

-----------------------

local UseMeta = { }

local function UseNamespace(self, Type)
	error(self, "Namespace using is not supported")
end

local function UseType(self, Type)
	return Alias(Type, Type:GetName())
end

function UseMeta.namespace( Type)
	error(UseMeta, "Namespace using is not supported")
end

function UseMeta.type(Type)
	error(UseMeta, "Type using is not supported")
end

function UseMeta.__call(self, Type)
	Assert.type(Type, self, " Invalid type for aliasing: ", Type)

	if Type:Type() == "Namespace" then
		return UseNamespace(self, Type)
	else
		return UseType(self, Type)
	end
end

x2c.MakeMetaObject(UseMeta, "Use")


local Generator = inheritsFrom(x2c.Classes.Exporter)

function Generator:GetObserver()
    local Observer = x2c.Classes.ObserverBroadcast.New()
	Observer:Add(self)
	return Observer
end

function Generator:post_struct_declartion(iType, Block, Generator)
    local Type = {
        LocalName = iType:LocalName(),
    }
    Block:Format ( "inline void {LocalName}_GetMemberInfo(::x2c::cxxpugi::StructureMemberInfoTable &output);", Type)
    Block:Format ( "inline void {LocalName}_GetWriteFuncs(std::unordered_map<std::string, std::function<void({LocalName} &self, const std::string &input)>> &funcs);", Type)
    Block:Format ( "inline void {LocalName}_GetReadFuncs(std::unordered_map<std::string, std::function<void(const {LocalName} &self,std::string &output)>> &funcs);", Type)
end

function Generator:struct_public(Type, Block, Generator)

    Block:Line { "static constexpr char *GetTypeName() { return ", quote(Type:GetName()), "; }" }

	Block:Line { "void GetMemberInfo(::x2c::cxxpugi::StructureMemberInfoTable &members) const {" }
	Block:BlockLine { "return ", Type:GlobalName(), "_GetMemberInfo(members);", }
	Block:Line "}"
	Block:Line { "void GetWriteFuncs(std::unordered_map<std::string, std::function<void(", Type:LocalName(), " &self, const std::string &input)>> &funcs) const {" }
	Block:BlockLine { "return ", Type:GlobalName(), "_GetWriteFuncs(funcs);", }
	Block:Line "}"
	Block:Line { "void GetReadFuncs(std::unordered_map<std::string, std::function<void(const ", Type:LocalName(), " &self, std::string &output)>> &funcs) {" }
	Block:BlockLine { "return ", Type:GlobalName(), "_GetReadFuncs(funcs);", }
	Block:Line "}"
end

function Generator:post_struct(Type, Block, Generator)
	local members = { }

	local add = function(name, decorated, default, type)
		members[#members + 1] = {
			name = name,
			decorated = decorated,
			default = tostring(default),
			type = type,
		}
	end

	local itfunc
	itfunc = function(member, nametree, decorated)
		local t = member.type

		while t.source_type do
			t = t.source_type
		end
		local name
		if nametree then
			name = nametree .. "." .. member.name
			decorated = decorated .. "." .. member.decoratedname
		else
			name = member.name
			decorated = member.decoratedname
		end
		if t:Type() ~= "Structure" then
			add(name, decorated, member.default or "", t)
		else
			for _,v in ipairs(t.fields) do
				itfunc(v, name, decorated)
			end
		end
	end

	for _,v in ipairs(Type.fields) do
		itfunc(v, nil, nil)
	end

	Block:BeginBlockLine { "inline void ", Type:LocalName(), "_GetMemberInfo(::x2c::cxxpugi::StructureMemberInfoTable &output) {" }
	Block:BeginBlockLine "auto table = ::x2c::cxxpugi::StructureMemberInfoTable {"
	for _,v in ipairs(members) do
		Block:Line { "::x2c::cxxpugi::StructureMemberInfo{ \"", v.name, "\", \"", v.default, "\", \"\", \"", v.type:GetName(), "\", },", }
	end
	Block:EndBlockLine "};"
	Block:Line { "table.swap(output);" }
	Block:EndBlockLine "}"

	Block:BeginBlockLine { "inline void ", Type:LocalName(), "_GetWriteFuncs(std::unordered_map<std::string, ",
		"std::function<void(", Type:LocalName(), " &Type, const std::string &input)>> &funcs) {" }
	for _,v in ipairs(members) do
		if v.type:Type() == "Enum" then
			Block:Line { "funcs[\"", v.name, "\"] = [](", Type:LocalName(), " &Type, const std::string &input) ",
				"{ ::x2c::cxxpugi::StringToEnum(input, Type.", v.decorated, "); };", }
		elseif v.type:Type() == "Type" then
			if v.type:GetName() == "string" then
				Block:Line { "funcs[\"", v.name, "\"] = [](", Type:LocalName(), " &Type, const std::string &input) ",
					"{ Type.", v.decorated, " = input; };", }
			else
				Block:Line { "funcs[\"", v.name, "\"] = [](", Type:LocalName(), " &Type, const std::string &input) ",
					"{ std::stringstream ss; ss << input; ss >> Type.", v.decorated, "; };", }
			end
		end
	end
	Block:EndBlockLine "}"

	Block:BeginBlockLine { "inline void ", Type:LocalName(), "_GetReadFuncs(std::unordered_map<std::string, ",
		"std::function<void(const ", Type:LocalName(), " &Type,std::string &output)>> &funcs) {" }
	for _,v in ipairs(members) do
		if v.type:Type() == "Enum" then
			Block:Line { "funcs[\"", v.name, "\"] = [](const ", Type:LocalName(), " &Type, std::string &output) ",
				"{ ::x2c::cxxpugi::EnumToString(Type.", v.decorated, ", output); };", }
		elseif v.type:Type() == "Type" then
			if v.type:GetName() == "string" then
				Block:Line { "funcs[\"", v.name, "\"] = [](const ", Type:LocalName(), " &Type, std::string &output) ",
					"{ output = Type.", v.decorated, "; };", }
			else
				Block:Line { "funcs[\"", v.name, "\"] = [](const ", Type:LocalName(), " &Type, std::string &output) ",
					"{ output = std::to_string(Type.", v.decorated, "); };", }
			end
		end
	end
	Block:EndBlockLine "}"
end

------------------------------------------------------------------------

function Generator:post_enum(Type, Block, Generator)
	Block:BeginStructure(Type:LocalName() .. "_TypeInfo")
	Block:MakeAlias("Type", Type:LocalName())
	Block:BeginBlockLine { "static constexpr char *GetTypeName() {" }
	Block:BlockLine { "return \"", Type:GetName(), "\";", }
	Block:EndBlockLine { "}", }
	Block:BeginBlockLine { "static bool GetValues(std::unordered_map<std::string, uint64_t> &values) {" }
	if Type.values then
		for i,v in ipairs(Type.values) do
			Block:Line { "values[\"", v.name, "\"] = static_cast<uint64_t>(", Type:LocalName(), "::", v.name, ");", }
		end
		Block:Line { "return true;", }
	else
		Block:Line { "return false;", }
	end
	Block:EndBlockLine { "}", }
	Block:EndStructure()
end

------------------------------------------------------------------------

--x2c.EnableExporter(Generator:Create())
x2c.RegisterExporter(Generator, "simplereflection")

return Generator

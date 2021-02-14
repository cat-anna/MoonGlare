
local BaseStructureGenerator = inheritsFrom(x2c.Classes.Exporter)

function BaseStructureGenerator:post_struct_declartion(iType, Block, Generator)
	local Type = {
		LocalName = iType:LocalName(),
	}
    Block:Format ("inline void {LocalName}_SetDefault({LocalName} &value);", Type)
end

function BaseStructureGenerator:struct_public(Type, Block, Generator)
    local iType = {
		LocalName = Type:LocalName(),
		GlobalName = Type:GlobalName(),
	}

    Block:Format "void ResetToDefault() {"
	Block:BlockFormat ( "return {GlobalName}_SetDefault(*this);", iType)
	Block:Format "}"
end

function BaseStructureGenerator:post_struct(Type, Block, Generator)
    Block:Line { "inline void ", Type:LocalName(), "_SetDefault(", Type:LocalName(), " &value) {" }
    Block:BeginBlock()
    for i,v in ipairs(Type.fields) do
        Generator:GenereateReset(v.type, Block, "value." .. v.decoratedname, v.default)
    end
    Block:EndBlock()
    Block:Line "}"
end

x2c.EnableExporter(BaseStructureGenerator:Create())

---------------------------------------------------------------------------------------------------

local FileInclude = inheritsFrom(x2c.Classes.Exporter)

function FileInclude:file_start(_, Block, Generator)
    --for i,v in ipairs(x2c.BaseFile.Imports) do
    --    Block:Line (v.FileName)
    --end
end

x2c.EnableExporter(FileInclude:Create())

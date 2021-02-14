
function x2c.ImportInputFiles()
    Import(x2c.inputfile)
end

local ImportMeta = { }
local ImportLevel = 0
local FileStack = { }

function ImportMeta.__call(self, importfn)

	print("import ", importfn)
	local fn = importfn:match("([^/\\]+)$") or importfn

	if x2c.importsByName[fn] then
		return
	end

	local prev
	if ImportLevel > 0 then
		prev = FileStack[ImportLevel]
	end

	local ffull

	if prev then
		ffull = prev.path .. importfn
	else
		ffull = importfn
	end

	fpath = ffull:match("(.*[/\\])")
	local fileinfo = {
		types = { },
		FileName = fn,
        Imports = { },
	}
	x2c.imports[#x2c.imports + 1] = fileinfo
    x2c.importsByName[fn] = fileinfo
    if not x2c.importsByLevel[ImportLevel] then
        x2c.importsByLevel[ImportLevel] = { }
    end

    local ilevel = x2c.importsByLevel[ImportLevel]
    ilevel[#ilevel + 1] = fileinfo

	info(string.format("Processing file %s (%s)", fn, ffull))
	local f, err = loadfile(ffull)

	if not f then
		error("Failed to load file ", fn, "\n", err)
	end

	ImportLevel = ImportLevel + 1
	FileStack[ImportLevel] = {
		import = importfn,
		name = fn,
		path = fpath,
		ffull = ffull,
		fileinfo = fileinfo,
	}

	if not x2c.settings.gen_all then
        x2c.exports[#x2c.exports + 1] = fileinfo
        fileinfo.Generate = ImportLevel == 1
    else
        fileinfo.Generate = true
	end

    if  x2c.CurrentFie then
        local Deps = x2c.CurrentFie.Imports
        Deps[#Deps + 1] = fileinfo
    end

    local prevfile = x2c.CurrentFie
    if not x2c.BaseFile then
        x2c.BaseFile = fileinfo
    end
	x2c.CurrentFie = fileinfo
	namespace ""
	f()

    x2c.CurrentFie = prevfile
	namespace ""

	FileStack[ImportLevel] = nil
	ImportLevel = ImportLevel - 1

	if not x2c.settings.gen_all then
	end
end

x2c.MakeMetaObject(ImportMeta, "Import")

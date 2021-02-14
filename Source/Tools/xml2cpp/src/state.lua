
local x2c = _G["x2c"]

function x2c.RegisterType(newtype, namespace)
	local cf = x2c.CurrentFie

	local tinfo = {
		value = newtype,
		namespace = namespace,
	}

	cf.types[#cf.types + 1] = tinfo
	if cf.Generate then
		x2c.Exporter:RegisterType(tinfo)
		x2c.ExportTypes[#x2c.ExportTypes + 1] = tinfo
	end

	namespace:Add(newtype)
end

function x2c.CheckStartingState()
	local argerror = false
	local help = function(...)
		print(...)
		argerror = true
	end

    if not x2c.inputfile then
        help("No input files")
    end
	if not x2c.outputfile then
		help("No outut file")
	end

	if argerror then
		x2c.PrintHelp()
        os.exit(1)
	end
end

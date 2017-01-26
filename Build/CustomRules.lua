--Moonglare build subscript

--rule "x2c"
--	display "xml2cpp"
--	fileExtension ".x2c"

function MoonGlare.X2CRule()

--filter 'files:**.x2c'
--	buildmessage 'Processing %{file.relpath} with xml2cpp'
--	buildcommands 'xml2cpp --input "%{file.relpath}" --output "%{cfg.objdir}%{file.basename}.x2c.h"'
--	buildoutputs '%{cfg.objdir}%{file.basename}.x2c.h"'

	filter 'files:**/Shared/x2c/**.x2c'
		buildmessage 'Processing %{file.name} with xml2cpp'
		buildcommands 'CALL xml2cpp --enable-all --input "%{file.relpath}" --output "%{cfg.objdir}%{file.basename}.x2c.h"'
		buildoutputs '%{cfg.objdir}%{file.basename}.x2c.h'


	filter {}
end
--[[

rule "VersionCompiler"
	display "Compiling version information"
	fileExtension ".verc"

	 buildmessage 'Compiling %(Filename) with MyCustomCC'
	 buildcommand 'verupdate --input=%(FullPath) --output=.\\ %(ProjectName)'
	 buildoutputs '%(ProjectName)Version.inc"'
	
]]

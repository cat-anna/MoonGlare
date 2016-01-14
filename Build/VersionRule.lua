--Moonglare build subscript

--[[

rule "VersionCompiler"
	display "Compiling version information"
	fileExtension ".verc"

	 buildmessage 'Compiling %(Filename) with MyCustomCC'
	 buildcommand 'verupdate --input=%(FullPath) --output=.\\ %(ProjectName)'
	 buildoutputs '%(ProjectName)Version.inc"'
	
]]

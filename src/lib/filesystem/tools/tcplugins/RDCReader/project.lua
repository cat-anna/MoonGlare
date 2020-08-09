if not os.is("windows") then
	return
end

group "tcplugins"
	project "RDCReader"
		location(dir.bin .. "tc/RDCReader")
		kind "SharedLib"
		targetdir(dir.bin .. "tc/RDCReader")
		
		links  {
			"StaticZLib",
		}
		files {
			"**",
			"../../core/**",
            "../common/**",
		}
		defines {
			"STARVFS_LOG_TO_SINK",
            "STARVFS_ENABLE_DEBUG_LOG",
		}
		
local outfile = dir.bin .. "/SVFSRemote"
		postbuildcommands {
			"echo Building RDCReader plugin installer...",
			"del /Q \"" .. dir.bin  .. "/RDCReader.zip\"",
			"zip " .. dir.bin .. "/RDCReader.zip pluginst.inf *.wcx*",
		}		

		filter "platforms:x32" 
			targetextension ".wcx"
             targetsuffix ""
		filter "platforms:x64" 
			targetextension ".wcx64"
             targetsuffix ""
			
		filter 'files:pluginst.inf'
			buildmessage 'Copying pluginst.inf'
			buildcommands 'cp "%{file.relpath}" "%{file.basename}.inf"'
			buildoutputs '%{cfg.objdir}%{file.basename}.inf'
			

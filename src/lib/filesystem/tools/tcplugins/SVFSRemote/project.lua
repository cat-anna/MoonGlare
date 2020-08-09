if not os.is("windows") then
        return
end

group "tcplugins"
	project "SVFSRemote"
		location(dir.bin .. "tc/SVFSRemote")
		kind "SharedLib"
		targetdir(dir.bin .. "tc/SVFSRemote")
		
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
			"echo Building SVFSRemote plugin installer...",
			"del /Q \"" .. dir.bin .. "/SVFSRemote.zip\"",
			"zip " .. dir.bin .. "/SVFSRemote.zip pluginst.inf *.wfx*",
		}		

		filter "platforms:x32" 
			targetextension ".wfx"
             targetsuffix ""
		filter "platforms:x64" 
			targetextension ".wfx64"
             targetsuffix ""
			
		filter 'files:pluginst.inf'
			buildmessage 'Copying pluginst.inf'
			buildcommands 'cp "%{file.relpath}" "%{file.basename}.inf"'
			buildoutputs '%{cfg.objdir}%{file.basename}.inf'
			

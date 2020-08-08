--MoonGlare build script

require("premake-export-compile-commands/export-compile-commands")

MoonGlare = {
    path = {
        root = path.normalize(os.getcwd() .. "../../../"),
        build_scripts = os.getcwd(),
    },
    tools = {
        bin2c = path.normalize(os.getcwd() .. "/bin2c.lua"),
    },
    feature_list = {}
}

function enable(what)
    MoonGlare.feature_list[what]()
end

function MoonGlare:start()
    if not _PREMAKE_VERSION then
        print "Premake5 not detected!"
        return
    end

    self.path.bin = self.path.root .. "/bin"
    self.path.project = self.path.bin .. "/" .. (_ACTION or "none")
    self.path.target = self.path.bin .. "/%{cfg.buildcfg}_%{cfg.architecture}"

    local workspace_name = "MoonGlareEngine"
    local build_config = self.path.root .. "/build_configuration.lua"

    if os.isfile(build_config) then
        self.configuration = dofile(build_config)
    else
        self.configuration = {}
    end
    
    local disable_projects = self.configuration.disable_projects or {}
    self.configuration.disable_projects = {}
    for _,v in ipairs(disable_projects) do 
        self.configuration.disable_projects[v] = true
    end

    print("Preparing workspace " .. workspace_name)
    local sol = workspace(workspace_name)
    
    print("Loading common configuration")
    dofile(MoonGlare.path.build_scripts .. "/common_config.lua")

    print("Preparing qt support")
    dofile(MoonGlare.path.build_scripts .. "/qt_support.lua")

    print("Loading projects")
    dofile(MoonGlare.path.build_scripts .. "/projects.lua")

    -- include(dir.lib .. "LibProjects.lua")
end

MoonGlare:start()

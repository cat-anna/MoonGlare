
-- require "/Scripts/oo"
-- require "/Scripts/EventUtils"
-- require "/Scripts/Stack"
-- require "/Scripts/Table"

-- local r = require "RuntimeConfiguration"
-- r.scene.firstScene = "..."
-- r.scene.loadingScene = "DefaultLoadingScene"
-- r.consoleFont = "Saira-Regular"

--[[@ [/BaseDataModule] Moonglare base module
@]]

print("BASE MODULE INIT")

require "/scripts/formatting"

local scenes = require("moonglare.scenes")
scenes:create_scene("moonglare_loading_scene", scenes.LOADING_SCENE_NAME)

local app = require("moonglare.application")
print(app.application_name)
print(app.compilation_date)
print(app.version_string)

-- local ctx = require("moonglare.lua_context")
-- print("MEM:", tostring(ctx.memory_usage))

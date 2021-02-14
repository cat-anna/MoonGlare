
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

require "base/scripts/formatting"

local scenes = require("moonglare.scenes")
scenes:create_scene("moonglare_loading_scene", scenes.loading_scene_name)

local app = require("moonglare.application")

print(app.application_name)
print(app.compilation_date)
print(app.version_string)

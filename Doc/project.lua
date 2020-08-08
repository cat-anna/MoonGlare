
-- group "Tools"
--     project "docs"
--         kind "Utility"
--         files "**"

--         -- MoonGlare.SetOutputDir("doc")

--         postbuildcommands {
--             [[lua "]] .. dir.build .. [[/DocGen.lua" --output=]] .. "%{cfg.targetdir}" ..  [[ --input=]] .. path.getabsolute(dir.src),
--         }
        

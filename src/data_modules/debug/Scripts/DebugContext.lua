--part of MoonGlare engine debug module

-- local DebugHooks = { }
-- local DEBUG_OBJECT_URI = "file:///Objects/DebugObject.xml"
-- function DebugHooks:SceneStateChangeEvent(event, arg)
	-- if arg.State == 1 then
		-- arg.Scene:SpawnChild(DEBUG_OBJECT_URI, "debug")
		--print "SceneStateChangeEvent Created"
	-- end
-- end

--[[
	local w = Inst.Window
	w:ReleaseMouse()
	w:SetPosition(5, 20)
]]

if not Debug then
	return
end

-- function TogglePhysicEnabled()
-- 	local val = not Debug.Flags.Physics.Enabled
-- 	if val then
-- 		print("Physics enabled")
-- 	else
-- 		print("Physics disabled")
-- 	end
-- 	Debug.Flags.Physics.Enabled = val
-- 	return 0;
-- end

-- function ToggleDebugDraw()
-- 	local val = not Debug.Flags.Physics.DebugDraw
-- 	if val then
-- 		print("DebugDraw enabled")
-- 	else
-- 		print("DebugDraw disabled")
-- 	end
-- 	Debug.Flags.Physics.DebugDraw = val;
-- 	return 0;
-- end

-- function ToggleRectTransformDebugDraw()
-- 	local val = not Debug.Flags.RectTransformComponent.DebugDraw
-- 	if val then
-- 		print("DebugDraw enabled")
-- 	else
-- 		print("DebugDraw disabled")
-- 	end
-- 	Debug.Flags.RectTransformComponent.DebugDraw = val;
-- 	return 0;
-- end

--part of MoonGlare engine debug module

function Debug_OnInit()
	if not Config.Debug then 
		
	--	return;
	end
	local w = Inst.Window
	w:ReleaseMouse()
	w:SetPosition(5, 20)
end

function TogglePhysicEnabled()
	local val = not Debug.Flags.Physics.Enabled
	if val then
		print("Physics enabled")
	else
		print("Physics disabled")
	end
	Debug.Flags.Physics.Enabled = val
	return 0;
end

function ToggleDebugDraw()
	local val = not Debug.Flags.Physics.DebugDraw
	if val then
		print("DebugDraw enabled")
	else
		print("DebugDraw disabled")
	end
	Debug.Flags.Physics.DebugDraw = val;
	return 0;
end

--Debug_OnInit()

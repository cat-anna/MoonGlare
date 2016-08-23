--part of MoonGlare engine debug module

function Debug_OnInit()
	if not Config.Debug then 
		
	--	return;
	end
	local w = Inst.Window
	--w:ReleaseMouse()
	w:SetPosition(5, 20)
end

function TogglePhysicEnabled()
	if Physics.Enable(false) == false then
		Physics.Enable(true);
		Log.Console("Physics enabled")
	else
		Log.Console("Physics disabled")
	end
	
	return 0;
end

function ToggleDebugDraw()
	if Physics.DebugDraw(false) == false then
		Physics.DebugDraw(true);
		Log.Console("DebugDraw enabled")
	else
		Log.Console("DebugDraw disabled")
	end
	return 0;
end

Debug_OnInit()

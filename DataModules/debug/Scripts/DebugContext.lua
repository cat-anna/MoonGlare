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
	if Physics.Enable(false) == false then
		Physics.Enable(true);
	end
	
	return 0;
end

function ToggleDebugDraw()
	if Physics.DebugDraw(false) == false then
		Physics.DebugDraw(true);
	end 
	return 0;
end



Inst.GlobalContext:Init([[ Debug_OnInit() ]])

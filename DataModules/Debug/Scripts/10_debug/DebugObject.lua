local DebugObject = Script:New("DebugObject")

function DebugObject:OnCreate()
	Settings.Input:RegisterKeySwitch("DebugHelp", "F1")

	Settings.Input:RegisterKeySwitch("DebugHelpPhysics", "F2")
	Settings.Input:RegisterKeySwitch("DebugHelpPhysicsDebugDraw", "F3")
	Settings.Input:RegisterKeySwitch("DebugHelpPhysicsRectTransformDraw", "F4")
end

function DebugObject:Step(data)
	if Input.DebugHelp == 3 then
		print "Internal Debug Keys:"
		print "F1 - Print debug help"
		print "F2 - En/Dis Physics"
		print "F3 - En/Dis Physics debug draw"
		print "F4 - En/Dis RectTransform"
	end

	if Input.DebugHelpPhysics == 3 then
		TogglePhysicEnabled()
	end
	if Input.DebugHelpPhysicsDebugDraw == 3 then
		ToggleDebugDraw()
	end
	if Input.DebugHelpPhysicsRectTransformDraw == 3 then
		ToggleRectTransformDebugDraw()
	end
end

-- GuiMouse script

local GuiMouse = oo.Inherit("/GUI/Gui.Base")

function GuiMouse:OnCreate()
	self.Transform = self:GetComponent(Component.RectTransform)

	local parent = self.Transform:GetParent()
	self.Parent = parent
	if not parent then
		return
	end

	local ptrans = parent:GetComponent(Component.RectTransform)
	self.Area = ptrans:FindChildByName("CenterCol")
	
	self.AreaTransform = self.Area:GetComponent(Component.RectTransform)
	self.Ctrl = self.Area:GetComponent(Component.Script)
	self.ScreenSize = ptrans.Size
	self.Transform.Position = Vec2(self.ScreenSize.x / 2, self.ScreenSize.y / 2),

	Settings.Input:RegisterKeySwitch("MouseClick", "Mouse Button Left")
end

function GuiMouse:OnDestroy()
end

function GuiMouse:Step(data)
	local Turn      = Input.Turn
	local LookAngle = Input.LookAngle

	local click		= Input.MouseClick == 1

	local T = self.Transform

	if click then
		self.Ctrl:OnMouseClick(T.ScreenPosition)
		return
	end

	if Turn == 0.0 and LookAngle == 0.0 then
		return
	end

	local vec = Vec2(Turn, LookAngle) + T.Position
	vec:Clamp(Vec2.Zero, self.ScreenSize)
	T.Position = vec
	local item = self.AreaTransform:FindChildAtPosition(vec)
	self.Ctrl:OnMouseMove(item, vec)
end

return GuiMouse

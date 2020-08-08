-- MoonGlare.About script

local MoonGlareAbout = oo.Inherit("ScriptComponent")

local playListUri = "file:///Scenes/MoonGlareAbout/PlayList.json"

function MoonGlareAbout:OnCreate()
	self.playListCtl = (require "PlayListCtl").New(self, playListUri)

	self.Transform = self.GameObject:GetComponent(Component.RectTransform)
	self.Transform.AlignMode = 0
	self.Size = self.Transform.Size

	self.LineSize = Vec2(self.Size.x, 0) + self.Transform:PixelToCurrent(Vec2(0, 40))
	self.YPos =  self.Transform:PixelToCurrent(Vec2(0, 350)).y
	self.Speed = 0.1
	if Config.Debug then
		self.Speed = 0.3
	end

	self:AddTextLine("MoonGlare Engine", 4, { Caption = { FontSize = 64 } })

	if Config.Debug then
		self:AddTextLine("Debug build")
		self:AddTextLine("Build: ${MoonGlare.BuildDate}")
	--	self:AddTextLine("Debug build")
	end
	self:AddTextLine("Version: ${MoonGlare.Version}")

	self:AddTextLine("TBD")

	local pos = self.Transform.Position
	pos.y = self.Size.y
	self.Transform.Position = pos
end

function MoonGlareAbout:OnDestroy()
end

function MoonGlareAbout:Step(data)
	local pos = self.Transform.Position

	pos.y = pos.y - self.Speed * data.TimeDelta

	if pos.y < -self.YPos then
		pos.y = self.Size.y
	end

	self.Transform.Position = pos

	if Input.Escape == 1 then
		Inst.SceneManager:LoadScene("MainMenu")
	end
end

function MoonGlareAbout:AddTextLine(Text, mul, attrs)
	mul = mul or 1.0
	local spawninfo = {
		Object = "file:///Scenes/MoonGlareAbout/MoonGlareAboutLine.epx",
		Name = "item",
	}

	local item = self.GameObject:LoadObject(spawninfo)
	if not item then
		print "error1"
		return
	end

	local Caption = item:GetComponent(Component.Text)
	local Transform = item:GetComponent(Component.RectTransform)
	if not Caption or not Transform then
		print "error2"
		return
	end
	if attrs and attrs.Caption then
		Caption:Set (attrs.Caption)
	end
	Caption:Set {
		Text = Text,
	}
	Transform.Size = self.LineSize * Vec2(1, mul)
	Transform.Position = Vec2(0, self.YPos )
	--Transform:Set {
	--	Size = ,
----		--Position = Vec2(0, self.YPos),
	--}
	self.YPos = self.YPos + self.LineSize.y * mul
end

function MoonGlareAbout:OnTimer()
end

return MoonGlareAbout

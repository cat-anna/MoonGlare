-- InfoBar script

local InfoBar = oo.Inherit("/GUI/Gui.Base")

function InfoBar:OnCreate()
	self:SetStep(false)
	self:SetInterval(1000)
	self.Transform = self:GetComponent(Component.RectTransform)
	local Lines = { }
	for i=0,1 do
		local name = string.format("Line%d", i)
		local line = self.Transform:FindChildByName(name)
		if not line then
			break
		end
		local txt = line:GetComponent(Component.Text)
		if not txt then
			break
		end

		Lines[i + 1] = {
			GameObject = line,
			Text = txt,
		}
		txt:Set {
			Text = name,
		}
	end
	self.Lines = Lines

	Lines[1].Text:Set { Text = "${MoonGlare.InfoLine}", }
end

function InfoBar:OnTimer(tid)
	local Lines = self.Lines
	if not Lines then
		return
	end
	Lines[2].Text:Set { Text = string.format("%.1f fps", 1.0 / Time.delta) }
end

return InfoBar

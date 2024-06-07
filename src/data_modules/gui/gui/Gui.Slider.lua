
local GuiSlider = oo.Inherit("/GUI/Gui.Base")

function GuiSlider:OnCreate()
    self.RectTransform = self.GameObject:GetComponent(Component.RectTransform)

    local CaptionGo = self.RectTransform:FindChildByName("Caption")
    self.Caption = CaptionGo:GetComponent(Component.Text)

end

function GuiSlider:PostInit()
    local txt = self.CallIfFunction(self.item.Caption, self.item, self)
    self.Caption:Set { Text = txt, }
    self.GameObject:SetName(txt)    
end

function GuiSlider:Step()
    self:SetStep(false)

    local itemdef = self.item

    if itemdef.OnCreate then
        self.Value = itemdef.OnCreate(self)
    end    
    if itemdef.OnGetValue then
        self.Value = itemdef.OnGetValue(self)
    end

    local BorderGo = self.RectTransform:FindChildByName("Border")
    -- BorderGo.Owner = self.GameObject
    self.Border = BorderGo:GetComponent(Component.RectTransform)
    local ActiveGo = self.RectTransform:FindChildByName("Active")
    -- ActiveGo.Owner = self.GameObject
    self.ActiveRT = ActiveGo:GetComponent(Component.RectTransform)
    self.ActiveImage = ActiveGo:GetComponent(Component.Image)
    local InactiveGo = self.RectTransform:FindChildByName("Inactive")
    -- InactiveGo.Owner = self.GameObject
    self.InactiveRT = InactiveGo:GetComponent(Component.RectTransform)
    self.InactiveImage = InactiveGo:GetComponent(Component.Image)

    --	local ImageGo = self.GameObject:FindChildByName("Image")
    --	self.Image = ImageGo:GetComponent(Component.Image)
    --	ImageGo.Owner = self.GameObject

    local itemdef = self.item
    self.Caption:Set {
        Text = self.CallIfFunction(itemdef.Caption, itemdef, self),
    }

    self:OnMouseLeave()
    self:RefreshValue()
end

function GuiSlider:OnMouseEnter(Ctrl, Reciver)
    local color = Vec4.White,

    --self.Image:Set {
    --	Color = color,
    --}
    self.Caption:Set {
        Color = color,
        FontSize = 32,
    }
    self.Hovered = true
    self:RefreshValue()
end

function GuiSlider:OnMouseLeave(Ctrl, Reciver)
    local color = Vec4(0.5, 0.5, 0.5, 1),

    --self.Image:Set{
    --	Color = color,
    --}
    self.Caption:Set {
        Color = color,
        FontSize = 30,
    }
    self.Hovered = false
    self:RefreshValue()
end

function GuiSlider:OnEnter(Ctrl, Reciver, MousePos)
    local itemdef = self.item
    if itemdef.Type ~= "Slider" then
        print "Invalid menut type!"
        return
    end

    local localpos = MousePos - self.Border.ScreenPosition
    local size = self.Border.Size
    self.Value = localpos.x / size.x

    self:RefreshValue()
    if itemdef.OnSetValue then
        itemdef.OnSetValue(self, self.Value)
    end
end

function GuiSlider:RefreshValue()
    local v = self.Value
    local size = self.Border.Size
    local w = size.x
    local h = size.y

    self.ActiveRT.Size = Vec2(w * v, h)
    self.InactiveRT.Size = Vec2(w * (1 - v), h)

    local hmod
    if self.Hovered then
        hmod = 0.4
    else
        hmod = 0.2
    end

    local av = hmod + v * 0.5
    self.ActiveImage:Set { Color = Vec4(av, av, av, 1), }
    local iv = hmod + (1 - v) * 0.5
    self.InactiveImage:Set { Color = Vec4(iv, iv, iv, 1), }
end

return GuiSlider

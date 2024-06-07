local GuiCheck = oo.Inherit("/GUI/Gui.Base")

function GuiCheck:OnCreate()
    self.RectTransform = self.GameObject:GetComponent(Component.RectTransform)
    
    local CaptionGo = self.RectTransform:FindChildByName("Caption")
    self.Caption = CaptionGo:GetComponent(Component.Text)
    local ImageGo = self.RectTransform:FindChildByName("Image")
    self.Image = ImageGo:GetComponent(Component.Image)

    self.Value = false
end

function GuiCheck:PostInit()
    local txt = self.CallIfFunction(self.item.Caption, self.item, self)
    self.Caption:Set { Text = txt, }
    self.GameObject:SetName(txt)    
end

function GuiCheck:Step()
    self:SetStep(false)

    local itemdef = self.item
    if itemdef.OnCreate then
        self.Value = itemdef.OnCreate(self)
    end    
    self.Caption:Set {
        Text = self.CallIfFunction(itemdef.Caption, itemdef, self),
    }
    self:OnMouseLeave()

    local itemdef = self.item

    if itemdef.OnGetValue then
        self.Value = itemdef.OnGetValue(self)
    end
    self:SetImagePos()
end

function GuiCheck:OnMouseEnter(Ctrl, Reciver)
    local color = Vec4.White

    self.Image:Set {
        Color = color,
    }
    self.Caption:Set {
        Color = color,
        FontSize = 32,
    }
end

function GuiCheck:OnMouseLeave(Ctrl, Reciver)
    local color = Vec4(0.5, 0.5, 0.5, 1),

    self.Image:Set{
        Color = color,
    }
    self.Caption:Set {
        Color = color,
        FontSize = 30,
    }
end

function GuiCheck:OnEnter(Ctrl, Reciver)
    local itemdef = self.item
    if itemdef.Type ~= "Check" then
        print "Invalid menut type!"
        return
    end
    self.Value = not self.Value
    self:SetImagePos()

    if itemdef.OnSetValue then
        itemdef.OnSetValue(self, self.Value)
    end
end

function GuiCheck:SetImagePos()
    if self.Value then
        self.Image:Set {
            Position = 1,
        }
    else
        self.Image:Set {
            Position = 0,
        }
    end
end

return GuiCheck

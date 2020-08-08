
local GuiList = oo.Inherit("/GUI/Gui.Base")

function GuiList:OnCreate()
    self.RectTransform = self.GameObject:GetComponent(Component.RectTransform)
end

-- function GuiCheck:PostInit()
--     local txt = self.CallIfFunction(self.item.Caption, self.item, self)
--     self.Caption:Set { Text = txt, }
--     self:SetName(txt)    
-- end

function GuiList:Step()
    self:SetStep(false)

    local itemdef = self.item
    self.ItemID = 1
    self.Loop = itemdef.Loop
    self.Values = self.CallIfFunction(itemdef.Values, self)

    self.Value = self.Values[self.ItemID]
    if itemdef.OnGetValue then
        local vx = itemdef.OnGetValue(self)
        for i, v in ipairs(self.Values) do
            if v.Value == vx then
                self.ItemID = i
                self.Value = v
                break;
            end
        end
    end

    local ValueObject = self.RectTransform:FindChildByName("Value")
    ValueRectTransform = ValueObject:GetComponent(Component.RectTransform)

    local CaptionObject = self.RectTransform:FindChildByName("Caption")
    
    local Left = ValueRectTransform:FindChildByName("Left")
    local Right = ValueRectTransform:FindChildByName("Right")

    -- LeftRectTransform = Left:GetComponent(Component.RectTransform)
    -- RightRectTransform = Right:GetComponent(Component.RectTransform)

    local ElemCaption = CaptionObject:GetComponent(Component.Text)
    ElemCaption:Set {
        Text = itemdef.Caption,
    }
    self.Caption = ValueObject:GetComponent(Component.Text)
    self.Caption:Set {
        Text = self.Value.Caption,
    }

    Left.Tag = -1
    -- Left.Owner = self.GameObject
    self.LeftImage = Left:GetComponent(Component.Image)
    Right.Tag = 1
    -- Right.Owner = self.GameObject
    self.RightImage = Right:GetComponent(Component.Image)

    local Def = {
        Color = Vec4(0.5, 0.5, 0.5, 1),
    }
    self.RightImage:Set(Def)
    self.LeftImage:Set(Def)
end

function GuiList:OnMouseEnter(Ctrl, Reciver)
    if not Reciver then
        --print "GuiList:OnMouseEnter nil"
        return;
    end
    if Reciver.Tag == 1 then
        self.RightImage:Set { Color = Vec4.White, }
    end
    if Reciver.Tag == -1 then
        self.LeftImage:Set { Color = Vec4.White, }
    end    
end

function GuiList:OnMouseLeave(Ctrl, Reciver)
    if not Reciver then
        --print "GuiList:OnMouseLeave nil"
        return;
    end

    local Color = Vec4(0.5, 0.5, 0.5, 1)
    if Reciver.Tag == 1 then
        self.RightImage:Set { Color = Color, }
    end
    if Reciver.Tag == -1 then
        self.LeftImage:Set { Color = Color, }
    end     
end

function GuiList:OnEnter(Ctrl, Reciver)
    local itemdef = self.item
    if itemdef.Type ~= "List" then
        print "Invalid menut type!"
        return
    end
    local iid = self.ItemID + Reciver.Tag
    local Count = #self.Values

    if self.Loop then
        if iid < 1 then
            iid = Count
        elseif iid > Count then
            iid = 1
        end
    else
        if iid < 1 then
            iid = 1
        elseif iid > Count then
            iid = Count
        end
    end
    self.ItemID = iid
    self.Value = self.Values[iid]
    self.Caption:Set {
        Text = self.Value.Caption,
    }
    if itemdef.OnSetValue then
        itemdef.OnSetValue(self, self.Value)
    end
end

return GuiList

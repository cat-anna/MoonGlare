
local GuiButton = oo.Inherit("/GUI/Gui.Base")

function GuiButton:OnCreate()
    self:SetStep(false)
    self.Caption = self:GetComponent(Component.Text)
    self:OnMouseLeave()
end

function GuiButton:PostInit()
    local txt = self.CallIfFunction(self.item.Caption, self.item, self)
    self.Caption:Set { Text = txt, }
    self.GameObject:SetName(txt)    
    -- print("test", self.test)
end

function GuiButton:OnMouseEnter(Ctrl, Reciver)
    -- Settings.Input:BeginCharMode(self.Entity)
    -- Settings.Input:CaptureKey(self.Entity)
    if not self.Enabled then
        return
    end
    self.Caption:Set {
        FontSize = 30,
    }
end

function GuiButton:OnMouseLeave(Ctrl, Reciver)
    -- Settings.Input:EndCharMode()
    if not self.Enabled then
        return
    end
    self.Caption:Set {
        FontSize = 32,
    }
end

-- function GuiButton:OnCharEvent(event)
--     local s = require "StaticStorage"
--     s.t = s.t or { }
--     s.v = s.v or { }
--     print(event.char, #s.t)
--     table.insert(s.t, event.char)
--     s.v[string.char(event.char)] = true
    
--     self.Caption:Set {
--         Text = string.char(event.char)
--     }
-- end
-- function GuiButton:OnKeyEvent(event)
--     print(event.key)
-- end

function GuiButton:OnEnter(Ctrl, Reciver)
    if not self.Enabled then
        return
    end

    local itemdef = self.item
    local t = itemdef.Type

    self.CallIfFunction(itemdef.OnClick, self)
    
    if t == "Action" then
        itemdef:Action(self)
        return
    end
    if t == "Menu" then
        Ctrl:PushMenu(self, itemdef)
        return
    end
    if t == "Back" then
        Ctrl:PopMenu(self)
        return
    end
end

return GuiButton

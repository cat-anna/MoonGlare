-- GuiCtrl script

local ElementURIs = {
	Button = "file:///GUI/Button.epx",
	List = "file:///GUI/List.epx",
	Check = "file:///GUI/Check.epx",
	Slider = "file:///GUI/Slider.epx",
}

local function CallIfFunction(Item, ...)
	if type(Item) == "function" then
		return Item(...)
	end
	return Item
end

ElementURIs.Back = ElementURIs.Button
ElementURIs.Menu = ElementURIs.Button
ElementURIs.Action = ElementURIs.Button
ElementURIs.Label = ElementURIs.Button

local GuiCtrl = oo.Inherit("/GUI/Gui.Base")

function GuiCtrl:OnCreate()
	self.MenuStack = Stack:Create()
	self.ForceRefresh = true
	self.RectTransform = self.GameObject:GetComponent(Component.RectTransform)
end

function GuiCtrl:OnDestroy()
end

function GuiCtrl:OnTimer(tid, handle)
end

function GuiCtrl:Step(data)
	if not self.Lines then
		local Lines = { }
		for i=0,9 do
			local name = string.format("Line%d", i)
			local line = self.RectTransform:FindChildByName(name)
			if not line then
				break
			end
			local txt = line:GetComponent(Component.Text)
			local transform = line:GetComponent(Component.RectTransform)
			Lines[i + 1] = {
				GameObject = line,
				Text = txt,
				RectTransform = transform,
			}
			if txt then
				txt:Set {
					Text = string.format("x%d", i),
				}
			end
		end
		-- print ("got " .. tostring(#Lines) .. " lines")
		self.Lines = Lines

		local TitleGo = self.RectTransform:FindChildByName("Title")
		self.TitleText = TitleGo:GetComponent(Component.Text)

		local Parent = self.RectTransform:GetParent()
		local ParentScript = Parent:GetComponent(Component.Script)
		self.Style = ParentScript.Style
		if not ParentScript.MenuDef then
			print "Invalid menu def!"
		else
			self:PushMenu(self.GameObject, ParentScript.MenuDef)
		end
	end

	if Input.Escape == 1 then
		self:PopMenu(self, nil)
	end

	if self.ForceRefresh then
		self:RefreshMenu()
	end
end

function GuiCtrl:ClearMenu()
	self.Hovered = nil
	for i,v in ipairs(self.Lines) do
		local c = v.RectTransform:GetFirstChild()
		if c then
			c:Destroy()
		else
			-- print(string.format("Invalid item at %d", i))
		end
	end
end

function GuiCtrl:RefreshMenu()
	self.ForceRefresh = false
	self:ClearMenu()

	local menu = self.MenuStack:top()
	if not menu then
		print "Nil menu def!"
		return
	end
	while type(menu) ~= "table" do
		print "Invalid submenu type on stack!"
		if self.MenuStack:empty() then
			print "menustack is empty!"
			return
		end
		self.MenuStack:pop()
		menu = self.MenuStack:top()
	end

	local Lines = self.Lines
	if #menu.SubMenu > #Lines then
		error(string.format("Invalid menu count! count:%d limit:%d", #menu.SubMenu, #Lines))
	end
	self.TitleText:Set {
		Text = CallIfFunction(menu.Caption, menu, self),
	}
	for i,v in ipairs(menu.SubMenu) do
		if v.Type == "Empty" then
		else
			local line = Lines[i]
			local spawninfo = {
				Object = ElementURIs[v.Type],
				Name = "item_" .. tostring(i),
				Tag = i,
				-- Script = {
				-- 	test = 5
				-- },
				RectTransform = {
					Position = Vec2(0, 0),
					Size = Vec2(0, 0),
				}
			}

			local item = line.GameObject:LoadObject(spawninfo)
			if not item then
				print("Spawn child failed!")
			else
				local script = item:GetComponent(Component.Script)
				if script then
					script.Enabled = v.Type ~= "Label"
					script.Ctrl = self
					script.item = v
					if script.PostInit then
						script:PostInit()
					end					
				end
			end
		end
	end
end

function GuiCtrl:OnItemMouseEnter(Receiver, Source)
	Source = Receiver
	
	local S
	for i=1,7 do
		S = Receiver:GetComponent(Component.Script)
		if S then 
			break 
		end
		local P = Receiver:GetComponent(Component.RectTransform)
		if not P then
			break;
		end
		Receiver = P:GetParent()
	end

	if not S or not S.OnMouseEnter then
		-- local Owner = S.Owner
		-- if Owner then
		-- 	self:OnItemMouseEnter(Owner, Source)
		-- end
		return
	end
	S:OnMouseEnter(self, Source)
end

function GuiCtrl:OnItemMouseLeave(Reciver, Source)
	Source = Reciver

	local S
	for i=1,5 do
		S = Reciver:GetComponent(Component.Script)
		if S then break end
		local P = Reciver:GetComponent(Component.RectTransform)
		if not P then
			break;
		end
		Reciver = P:GetParent()
	end

	if not S or not S.OnMouseLeave then
		-- local Owner = S.Owner
		-- if Owner then
		-- 	self:OnItemMouseLeave(Owner, Source)
		-- end
		return
	end
	S:OnMouseLeave(self, Source)
end

function GuiCtrl:OnMouseMove(MouseObject, Position)
	local Hovered = self.Hovered
	self.MousePosition = Position
	if not MouseObject and not Hovered then
		return
	end

	if Hovered then
		if Hovered ~= MouseObject then
			self:OnItemMouseLeave(Hovered)
		else
			return
		end
	end

	self.Hovered = MouseObject
	if MouseObject then
		self:OnItemMouseEnter(MouseObject)
	end
end

function GuiCtrl:OnMouseClick(MousePos)
	Source = self.Hovered
	Reciver = self.Hovered

	if not Reciver then
		return
	end

	local S
	for i=1,5 do
		S = Reciver:GetComponent(Component.Script)
		if S then break end
		Reciver = Reciver:GetComponent(Component.RectTransform):GetParent()
	end

	if not S or not S.OnEnter then
		-- local Owner = S.Owner
		-- if Owner then
		-- 	return self:OnMouseClick(MousePos, Owner, Source)
		-- end
		return
	end

	S:OnEnter(self, Source, MousePos)
end

function GuiCtrl:PushMenu(Sender, itemdef)
	self.MenuStack:push(itemdef)
	if itemdef.OnEnter then
		itemdef:OnEnter(self)
	end
	self.ForceRefresh = true
end

function GuiCtrl:PopMenu(Sender)
	local top = self.MenuStack:top()
	if top.OnExit then
		top:OnExit(self)
	end
	self.MenuStack:pop()
	if self.MenuStack:empty() then
		Inst.Scene:SetFinishedState()
		Inst.Engine:ChangeScene()
		return
	end
	self.ForceRefresh = true
end

function GuiCtrl:Invalidate()
	self.ForceRefresh = true
end

return GuiCtrl

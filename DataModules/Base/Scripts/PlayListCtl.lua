local class = oo.Class()

function class:Init(Owner, playListUri)
    self.Owner = Owner

    Owner.OnSoundStreamFinishedEvent = function(_, ev)
        self:OnSoundStreamFinishedEvent(ev)
    end

    Owner.GameObject:CreateComponent(Component.SoundSource)
    local ss = Owner:GetComponent(Component.SoundSource)
    ss:Stop()
    ss.Loop = false
    if playListUri then
        self:SetPlayList(playListUri) 
    end    
end

function class:SetPlayList(playListUri)
	local fs = require "FileSystem"
	local pl = fs:ReadJSON(playListUri)
    self.playlist = pl.List
    self.Random = pl.Mode == "Random"
    self:OnSoundStreamFinishedEvent()
end

function class:OnSoundStreamFinishedEvent(ev)
    local ss = self.Owner:GetComponent(Component.SoundSource)
    local f = table.remove(self.playlist, 1)
    --TODO: random
    ss.File = f
    ss:Play()
    table.insert( self.playlist, f )
end

return class

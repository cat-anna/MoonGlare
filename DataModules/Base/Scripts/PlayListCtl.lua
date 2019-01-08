local class = oo.Inherit("ScriptComponent")

class.playListFile = nil --#[uri] Play list file uri

function class:OnCreate()
    local ss = self:GetComponent(Component.SoundSource)
    if not ss then
        self.GameObject:CreateComponent(Component.SoundSource)
        ss = self:GetComponent(Component.SoundSource)
    end

    if self.playListFile then
        self:SetPlayList(self.playListFile)
    end
end

function class:Init(Owner, playListUri)
    self.Owner = Owner

    -- ss:Stop()
    ss.loop = false
    if playListUri then
        self:SetPlayList(playListUri) 
    end    
end

function class:Step(playListUri)
end

function class:SetPlayList(playListUri)
    self.playListFile = playListUri
	local fs = require "FileSystem"
    local pl = fs:ReadJSON(playListUri)
    self.playlist = pl.List
    self.random = pl.Mode == "Random"
    self:SetTimeout(0)
end

function class:OnTimer(cookie)
    self:OnSoundStreamFinishedEvent()
end

function class:OnSoundStreamFinishedEvent(ev)
    print("OnSoundStreamFinishedEvent")
    local ss = self:GetComponent(Component.SoundSource)
    local f = table.remove(self.playlist, 1)
    --TODO: random
    ss.file = f
    ss:Play()
    table.insert( self.playlist, f )
end

return class

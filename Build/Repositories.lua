--Moonglare main build subscript 

MoonGlare.Repositories = {
	build = {
		url = "git@smaug:~/projects/games/moonglare.build.git",
		name = "Build system",
		folder = "build",
		gitclone = false,
		gitignore = true,
	},
	source = {
		url = "git@smaug:~/projects/games/moonglare.core.git",
		name = "Engine sources",		
		folder = "source",
		gitclone = true,
		gitignore = true,
	},
	ifs = {
		url = "git@smaug:~/projects/libs/InternalFileSystem.git",
		name = "InternalFileSystem library",		
		folder = "libs/InternalFileSystem",
		gitignore = true,
		gitclone = true,
	},
}

MoonGlare.Folders = {
	{
		folder = "modules",
		gitignore = false,
	},
	{
		folder = "libs",
		gitignore = true,
	},
	{
		folder = "logs",
		gitignore = true,
	},
	{
		folder = "bin",
		gitignore = true,
	},
}

local function pexecute(cmd)
	print(cmd)
	os.execute(cmd)
end

local function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function MoonGlare.WriteGitIgnore()
	if file_exists(".gitignore") then
		print("Gitignore script exists. Skipping creation.")
		return
	end

	print("Writting gitignore")
	local f = assert(io.open(".gitignore", "w+"))
	
	local key
	local item
	
	for key,item in pairs(MoonGlare.Repositories) do
		if item.gitignore then
			f:write(item.folder .. "/\n")
		end
	end
	
	for key,item in pairs(MoonGlare.Folders) do
		if item.gitignore then
			f:write(item.folder .. "/\n")
		end
	end

	-- f:write("libs/\n");
	-- f:write("bin/\n");
	
	f:close()
end	

function MoonGlare.CloneRepositories()
	local key
	local item
	for key,item in pairs(MoonGlare.Repositories) do
		if item.gitclone then
			local cmd = "git clone " .. item.url .. " " .. item.folder
			print("Clonning " .. item.name)
			pexecute(cmd)
		end
	end
end	

function MoonGlare.BuildBasicPremake()
	if file_exists("premake5.lua") then
		print("Premake script exists. Skipping creation.")
		return
	end
	
	print("Building basic premake script")
	local f = assert(io.open("premake5.lua", "w"))
	f:write([[
--MoonGlare Engine premake5 build script generator

include "build/MoonGlare.lua"

MoonGlareSolution "moonglare"
	startproject "moonglare"
		
	project "moonglare"
		kind "ConsoleApp"
		location "bin/moonglare"
		SetPCH { hdr = "pch.h", src = "source/pch.cpp", }	
		defines { "_BUILDING_ENGINE_", }
		defines ( Engine.Defines )
		files(Engine.Source)
		Features.ApplyAll(Engine.Features)
		prebuildcommands { 
			"verupdate --input=..\\..\\Source\\Version.xml --outputdir=..\\..\\Source\\Engine\\Core MoonGlareEngine",
		}
		ModuleManager:addLibs()
		ModuleManager:addSources()			
		filter "configurations:Debug"
			Features.ApplyAll(Engine.Debug.Features)
			defines(Engine.Debug.Defines)
		filter "configurations:Release"
			Features.ApplyAll(Engine.Release.Features)
			defines(Engine.Release.Defines)
			
			
	]])
	f:close()
end	

function MoonGlare.CreateFolders()
	print("Building workspace")
	local key
	local item
	for key,item in pairs(MoonGlare.Folders) do
		pexecute("mkdir " .. item.folder)
	end
end


function MoonGlare.GitInit()
	print("Initializing git")
	pexecute("git init")
	pexecute("git add .gitignore premake5.lua")
	pexecute("git commit -m \"MoonGlare workspace clone\"")
end

function MoonGlare.Initialize()
	MoonGlare.WriteGitIgnore()
	MoonGlare.CreateFolders()
	MoonGlare.CloneRepositories()
	MoonGlare.BuildBasicPremake()
	MoonGlare.GitInit()
end
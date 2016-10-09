
Libraries{
	libs = {
		{
			name = "OrbitLogger",
			remotes = {
				origin = "git@smaug.lan:~/projects/libs/OrbitLogger.git",
			},
			branch = "master",
			projects = {
				"src/library.lua",
			}
		},
		{
			name = "StarVFS",
			remotes = {
				origin = "git@smaug.lan:~/projects/libs/StarVFS.git",
			},
			branch = "master",
			projects = {
				"core/project.lua",
				"libs/project.lua",
			}
		},
		{
			name = "libSpace",
			remotes = {
				origin = "git@smaug.lan:~/projects/libs/libSpace.git",
			},
			branch = "master",
			projects = {
				"src/library.lua",
			}
		},
	},
}

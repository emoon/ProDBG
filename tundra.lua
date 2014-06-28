Build {

	Units = "units.lua",

	Configs = {
		{
			Name = "macosx-mono",
			DefaultOnHost = "macosx",
			Tools = { "mono" },
		},
		{
			Name = "linux-mono",
			DefaultOnHost = "linux",
			Tools = { "mono" },
		},
		{
			Name = "win32-dotnet",
			DefaultOnHost = "windows",
			Tools = { "dotnet" },
		},
	},
}



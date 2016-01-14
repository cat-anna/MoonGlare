-- Moonglare module build script

	define_module { 
		name = "MenuScene",
		type = "module",		
		
		files = { "*.cpp", "*.h", },
		defines = { },
		
		apigen = true,
		api = {
			MenuItemInfo = {
				Inherit = { "RootClass" },
				ApiCall = {
				},
				Eventes = {
				}
			},
		
			ListMenuItem = {
				Inherit = { "MenuItemInfo" },
				ApiCall = {
					GetValue = {
					    When = { "At menu item show" },
                        Parameters = { 
                            [1] = { name = "MenuItem", type = "MenuItemInfo", description = "", },
                            [2] = { name = "MenuScene", type = "MenuScene", description = "", },
                            [3] = { name = "ItemId", type = "int",  description = "", },
                            [4] = { name = "CurrentValue", type = "int", description = "", },
                            Return = { ignored = false, description = "", },
                        }
					},
					SetValue = {
					
					},
				},
				Events = {
				},
			},
		},
	}

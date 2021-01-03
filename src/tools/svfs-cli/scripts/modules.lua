

Help.Register { Command="load_module", Brief="load module", Usage="load_module(module_class, [args])" }
function load_module(module_class, opt_args)
	opt_args = opt_args or {
	}
	opt_args.module_manager = module_manager
	return StarVfs:LoadModule(module_class, opt_args)
end


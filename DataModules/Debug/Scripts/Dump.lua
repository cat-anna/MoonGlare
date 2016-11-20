--part of MoonGlare engine debug module

function Dump_GlobalCtx() 
	return Utils.Serialize(global, true)
end 

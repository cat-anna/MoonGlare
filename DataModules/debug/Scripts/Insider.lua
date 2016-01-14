
Insider = {
	EnumerateLuaTable = function (element, enumerator)
		if not element then
			return 0
		end
		for k,v in pairs(element) do
			enumerator:LuaElement(k, v);
		end
		return 0;
	end,
-----------------------------------------------------------	
}

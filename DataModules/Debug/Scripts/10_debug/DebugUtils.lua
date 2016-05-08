--part of MoonGlare engine debug module

DebugUtils = {
	DumpApi = function(obj)
			local t = getmetatable(obj);
			local f = function (t, f)
					local p = nil;
					for n,v in pairs(t) do 
						if (type(n) == "string") and (not string.find(n, "__")) then
							print(n, " = ", v);
						end;
						if n == "__parent" then 
							p = v;
						end;
					end				
					if p == nil then 
						return; 
					end;
					f(p, f);
				end;
			f(t, f);
		end,
-----------------------------------------------------------
}

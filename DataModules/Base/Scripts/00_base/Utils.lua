--part of MoonGlare engine internal module

Utils = Utils or {}

function Utils.ForEach(it, func, param)
	while not it:Ended() do 
		func(it:Get(), param);
		it:Next(); 
	end
end;

function Exit() 
	Inst.Application:Exit();
	return 0; 
end;

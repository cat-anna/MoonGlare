
Help.Register { Command="play", Brief="Start playing", Usage="play(FILE[, start[, release_on_stop]])" }
function play(p, start, release_on_stop)
	return SoundSystem:open(p, start or true, release_on_stop or true)
end

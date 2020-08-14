
Help.Register { Command="exit", Brief="exit from cli", Usage="exit([code])"}
function exit(code)
    code = code or 0
    Cli:Exit(code)
end

Help.Register { Command="q", Brief="exit from cli", Usage="q([code])"}
q = exit

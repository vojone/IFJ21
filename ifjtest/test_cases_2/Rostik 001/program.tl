require "ifj21"

function foo(a : string, b : integer, c : number, d : nil) : string, integer, number, nil
    write (a, b, c, d, "\n")
    return nil, nil, nil, nil
end

function main()
	local a : string
	local b : integer
	local c : number
	local d : nil
	a, b, c, d = foo(nil, nil, nil, nil)    
	write (a, b, c, d, "\n")
end

main()

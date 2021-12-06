require "ifj21"

function foo():string,integer,number
end


function main()
	local a : string = "314"
	local b : integer = 314
	local c : number = 3.14
	write(a, "\n", b, "\n", c, "\n")
	a, b, c = foo()
	write (a, "\n", b, "\n", c, "\n")
end

main()

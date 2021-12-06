require "ifj21"

function concat (x : string, y : string) : string, integer
	return x .. y, 0 
end

function main()
	local x : string
	local ret: integer
	x, ret = concat("ahoj", "svete")
	write(x, ret, "\n")
end

main()

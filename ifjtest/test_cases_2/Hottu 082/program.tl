require "ifj21"
function main()
	local a : integer = 1000
	while a > 0 then
		local b : string = "a"
		local c : integer = a
		local b..c : integer = 2
		write(b..c)
		a = a - 1
	end
end
main()
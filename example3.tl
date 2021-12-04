require "ifj21"

function foo(f: integer) :integer, string
end

function main()
	local a : integer = foo(foo(7))
end

main()

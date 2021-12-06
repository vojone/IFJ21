require "ifj21"

global foo : function(string):string

function main()
	local a : string = foo("hello")
	write(a, "\n")
end

function foo(str : string): string
	return (str.." there")
end

main()

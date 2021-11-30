require "ifj21"
global foo2 : function()

function main()
	foo2()
end

function foo2()
	local a : integer = 2
	write("Hodnota A je: ", a , "\n")
end
main()
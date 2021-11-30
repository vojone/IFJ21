require "ifj21"

function main()
	local a:string = "hello"
	local b:string = "there!"

	local x : integer 
	x = #"help" * #a + #b + #a * #(a..b)
	write(x, "\n")
end

main()

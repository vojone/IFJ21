require "ifj21"
function main()
	local a : string = "Hello World, how are you?"
	local b : number = 7.0
	local c : number = 11.0
	local d : string
	d = substr(a,b,c)
	write(d, "\n")
end
main()
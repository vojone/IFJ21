require "ifj21"

function main()
	local s : string = "Hello" .. " there\n"
	local t : string = "General" .. "\032" .. "Kenobi" .. "!\010"
	local u : string = s .. t
	write(u)
end

main()

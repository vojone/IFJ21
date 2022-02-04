require "ifj21"
function func(a : integer, b : integer) : integer
    return a + b
end

function main()
    local a : integer = 1
    local b : integer = 2
    local c : integer
    c = func(a,b)
    write("Hodnota C je: ", c , "\n")
end
main()
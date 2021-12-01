require "ifj21"
function func(a : integer, b : integer) : integer, integer
    return a, b
end
function main()
    local a : integer = 1
    local b : integer = 3
    local c : integer
    local d : integer
    c, d = func(a,b)
    write("Hodnota c: ", c," hodnota d: ", d ,"\n")
end
main()
require "ifj21"
function main()
    local a : integer = 1
    local b : integer = 2
    local c : integer = 3
    local d : integer = 4

    a, b, c, d = d, c, b, a, d

    write(a, b, c, d, "\n")
end
main()

require "ifj21"
function func(a : string, b : string, c : integer) : integer
    a = b
    return c
end

function main()
    local a : integer = 1
    local b : string = 2
    func(a,b)
end
main()
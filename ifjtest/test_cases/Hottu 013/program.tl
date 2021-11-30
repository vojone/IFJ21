require "ifj21"
function func(a : integer, b : string) : integer
    b = "s"
    return a
end

function main()
    local a : integer = 1
    local b : integer = 2
    func(a,b)
end
main()
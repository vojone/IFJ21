require "ifj21"

function f1() : integer, integer
    return 1,2
end

function f2() : integer, number
    return 3,4
end

function main()
    local a : integer
    local b : number

    a, b = f1(), f2()
    write(a, b)
    a, b = f1()
    write(a, b)
    a, b = f1(), 3
    write(a, b)
    
    local c : integer
    a, c, b = f1(), f2()
    write(a, c, b, "\n")

end
main()

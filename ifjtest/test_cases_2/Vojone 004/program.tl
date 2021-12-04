require "ifj21"

function bar() : integer, number
    write("Bar\n")
    return 1, 2
end

function foo() : integer, integer
    write("Foo\n")
    return 3, 4
end

function main()
    local a : number = 0
    local b : number = 0
    local c : integer = 0

    a, b, c = bar(), foo()

    write(a, b, c, "\n")
end
main()

require "ifj21"

function bar() : integer, integer
    write("Bar\n")
    return 1, 2
end

function foo() : integer, integer
    write("Foo\n")
    return 3, 4
end

function main()
    local a : number = 8e+123
    local b : integer = 1
    local c : number = 0.123e-123

    a, b, c = bar(), foo()

    write(a, b, c, "\n")
end
main()

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
    local b : integer = 0
    local c : number = 0

    a, b, c = tointeger(bar()), foo()

    write(a, b, c, "\n")
end
main()

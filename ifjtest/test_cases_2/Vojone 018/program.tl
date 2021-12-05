require "ifj21"

function foo() : integer
    write("Foo")
    return 1
end

function bar() : integer
    write("Bar")
    return 2
end

function baz() : integer, integer
    return foo(), bar()
end

function main()
    local a : integer
    local b : number

    a,b = baz()

    write(a, tointeger(b), "\n")

end

main()

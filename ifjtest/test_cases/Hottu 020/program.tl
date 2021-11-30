require "ifj21"
function foo(i : integer) : string
    local c : string = "s"
    return c
end
require "ifj21"
function main()
    local a : integer = nil
    local b : string
    b = foo(a)
    write("Hodnoty a,b jsou: ", a, b,"\n")
end
main()
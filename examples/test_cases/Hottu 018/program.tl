require "ifj21"
function concatenate(c : string, d : string) : string, integer
    return c .. d, 1
end
function main()
    local a : string
    local b : integer
    a,b = concatenate("Hello", "World")
    write(a , " " , b , "\n")
end
main()
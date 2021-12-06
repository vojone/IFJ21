require "ifj21"

function fn(a:integer):integer
    a = a+1
    return a
end

function main(a:integer)
    local b:integer = fn(a)
    write(a, b, "\n")
end

main(2)

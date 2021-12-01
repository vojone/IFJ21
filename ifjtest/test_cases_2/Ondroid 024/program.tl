require "ifj21"

function fn(): integer, number, string
end

function main()
    local a: integer = 1
    local b: number = 1
    local c: string = "hello"

    a, b, c = fn()

    if a == nil then
        write("yes\n")
    else
        write("no\n")
    end

    if b == nil then
        write("yes\n")
    else
        write("no\n")
    end

    if c == nil then
        write("yes\n")
    else
        write("no\n")
    end
end

main()

require "ifj21"

function main()
    local x:integer = 5
    local y:number = 4.9
    local z:string = "hello there"
    local n:nil = nil

    if x == n then
        write("yes\n")
    else
        write("no\n")
    end

    if y == n then
        write("yes\n")
    else
        write("no\n")
    end

    if z == n then
        write("yes\n")
    else
        write("no\n")
    end

    if n == n then
        write("yes\n")
    else
        write("no\n")
    end
end

main()

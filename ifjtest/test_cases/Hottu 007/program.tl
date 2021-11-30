require "ifj21"
function main()
    local a : integer = 1
    local b : integer = 2
    write("A(1) před IF = ", a ,"\n")
    if b == 2 then
        local a : integer = 3
        write("A(3) v = ", a ,"\n")
    end
    write("A(1) za IF = ", a ,"\n")
end
main()
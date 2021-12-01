require "ifj21"
function main()
    local a : integer = 0
    while a < 10 do
        local b: integer = 0
        a = a + 1
        write(a,b,"\n")
        while b < 10 do
            local c: integer = 99
            a = a + 1
            write(a,c,"\n")
            b = b + 1
        end 
    end
end
main()
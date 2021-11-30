
-- function foo()
--     local a: integer = 10
--     local b: integer = nil
--     write(b)
-- end
-- foo()

-- local b : integer = val()

-- function add(a:integer, b:integer) : integer, integer
--     return 2, 3
-- end

require "ifj21"

function factorial(n : integer) : integer
    local n1 : integer = n - 1

    if n < 2 then
        return 1
    else
        local tmp : integer = factorial(n1)
        return n * tmp
    end
end

function main()
    write("Zadejte cislo pro vypocet faktorialu: ")
    local a : integer = readi()

    if a ~= nil then
        if a < 0 then
            write("Faktorial nejde spocitat!", "\n")
        else
            local vysl : integer = factorial(a)
            write("Vysledek je ", vysl, "\n")
        end
    else
        write("Chyba pri nacitani celeho cisla!\n")
    end
end

main()



-- function main()
--     write(3.3//3)
-- end
-- main()

-- main()
-- function g()
--     local a : integer = 8 
--     if a == 8 then
--         local a : integer = a+2 --[[ to vnitřní 'a' by mělo být taky8]]
--         write(a)
--     end
-- end
-- g()

-- function g() 
--     local a : integer = add(6,2)
--     if a == 8 then
--         -- local a : integer = a --[[ to vnitřní 'a' by mělo být taky8]]
--         write(a)
--     end
-- end
-- g()

-- function matrix(imax :integer, jmax:integer )
--     local i : integer = 1
--     local j : integer = 1
--     while i <= imax do
--         write("\n")
    
--         while j <= jmax do
--             write(i,j)
            
--             write("\t")
--             j = j + 1

--         end
--         i = i + 1
--         j = 1

--     end
-- end
-- matrix(11,9)

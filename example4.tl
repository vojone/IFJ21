-- -- write(reads())

-- function fdoesntworko() : string, string, number, integer
--     local b : string
--     return fdoesntworko()
-- end

-- function print(a : integer, b:integer)
--     write(a,b)
-- end

-- function val() : integer, integer
--     return 1,2
-- end

-- a, b = val()

-- write( 10 // 1 )




-- local b : integer = val()

function add(a:integer, b:integer) : integer
    return a+b
end

function g() 
    local a : integer = add(6,2)
    if a == 8 then
        -- local a : integer = a --[[ to vnitřní 'a' by mělo být taky8]]
        write(a)
    end
end
g()

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
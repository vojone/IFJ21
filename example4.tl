function g() 
    local a : integer = 8 
    if a == 8 then
        local a : integer = a --[[ to vnitřní 'a' by mělo být taky 8]]
        write(a)
    end
end

write(substr(reads(),readn(),readn()))



-- function substr2(str : string, begin: number, finish:number) : string
--     local res : string = ""
--     local si : integer = tointeger(begin)
--     local ei : integer = tointeger(finish)
--     -- local i : integer = 0;
--     while si <= ei do
--         res = res..chr(ord(str,si))
--         si = si + 1
--     end
--     return res
-- end

-- write(substr2(reads(),readn(),readn()))

-- require "ifj21"

-- function modulo(a : integer, b : integer) : integer
--     return a - b * (a // b)
-- end

-- function main()
--     -- local key : string = "xplagi0b"

--     write("Enter a message to encode:\n")
--     local msg : string = reads()

--     local forward_shift : integer = ord(msg, 2)
--     local back_shift : integer = ord(msg, 3)

--     local alpha_start : integer = ord("a", 1)
--     local alpha_end : integer = ord("z", 1)

--     forward_shift = forward_shift - alpha_start + 1
--     back_shift = 0 - (back_shift - alpha_start) - 1


--     local i : integer = 1
--     local c : integer

--     local shifted_char : string

--     local modulo_result : integer

--     local result : string = ""

--     while i <= #msg do
--         c = ord(msg, i)

--         if c < alpha_start then
--             -- Pokud mate break pouzijte break (Jirka Babica 2021)
--             i = #msg + 1
--         else
--             modulo_result = modulo(i, 2)
            
--             if modulo_result == 0 then
--                 c = c + back_shift
--             else
--                 c = c + forward_shift
--             end

--             if c < alpha_start then
--                 c = c + 26
--             else end

--             if c > alpha_end then
--                 c = c - 26
--             else end

--             shifted_char = chr(c)
--             result = result .. shifted_char

--             i = i + 1
--         end
--     end

--     write(result, "\n")
-- end

-- main()
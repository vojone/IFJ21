require "ifj21"

function main()
    local a : integer = 1

    while a <= 3 do
        write("outside: ", a, "\n")

        local b : integer = 1

        while b <= 3 do
            write("inside: ", b, "\n")
            b = b + 1
        end

        a = a + 1
    end
end

main()

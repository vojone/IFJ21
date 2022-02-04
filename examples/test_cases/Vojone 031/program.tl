require "ifj21"

function get_avg(sum : number, n : integer) : number
    return sum/n
end

function main()
    local a : number = 0.0
    local b : number
    local c : number
    local total : integer = 0

    while total < 20 do
        local current : number = readn()
        a = a + current

        if b == nil then
            b = current
        end

        if current < b then
            b = current
        end

        if c == nil then
            c = current
        end

        if current > c then
            c = current
        end

        total = total + 1
    end

    local sum : number
    local avg : number
    local min : number
    local max : number

    sum, avg, min, max = a, get_avg(a, total), b, c

    write(sum, " ", avg, " ", min, " ", max, "\n")
end
main()

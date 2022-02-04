require "ifj21"

function sqrt(a:number) : number
    local result : number = a
    local last_result : number = a
    local counter : integer = 0

    while counter < 1000 do
        result = 1/2*(last_result + a/last_result)
        counter = counter + 1
        last_result = result
    end
    
    return result
end

function main()
    local _a : integer = readi()
    local _b : integer = readi()

    local c2 : number = _a^2 + _b^2
    local c : number = sqrt(c2)
    
    write(tointeger(c), "\n")
end
main()

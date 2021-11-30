require "ifj21"

function main()
    local a:integer = 5
    local b:number = 4.9
    local c:number = 5.1

    if a >= a then
        write("ano,")
    else
        write("ne,")
    end

    if a >= b then
        write("ano,")
    else
        write("ne,")
    end

    if a >= c then
        write("ano,")
    else
        write("ne,")
    end

    if b >= a then
        write("ano,")
    else
        write("ne,")
    end

    if b >= b then
        write("ano,")
    else
        write("ne,")
    end

    if b >= c then
        write("ano,")
    else
        write("ne,")
    end

    if c >= b then
        write("ano,")
    else
        write("ne,")
    end

    if c >= a then
        write("ano,")
    else
        write("ne,")
    end

    if a >= 5.0 then
        write("ano,")
    else
        write("ne,")
    end

    if a >= 4.9 then
        write("ano,")
    else
        write("ne,")
    end

    if a >= 5 then
        write("ano,")
    else
        write("ne,")
    end

    if a >= 6 then
        write("ano\n")
    else
        write("ne\n")
    end
end

main()

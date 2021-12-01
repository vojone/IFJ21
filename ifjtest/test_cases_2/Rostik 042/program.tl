require "ifj21"

function strfn():string,string,string
    return "hello there", "", nil
end

function intfn():integer,integer
    return 4, nil
end

function numfn():number,number,number,number,number,number,number,number
    return 1.1, 1, 1.1e1, 1.1e+1, 1.1e-1, 1e1, 1e+1, 1e-1
end

function main()
    local str1 : string
    local str2 : string
    local str3 : string
    local int1 : integer
    local int2 : integer
    local num1 : number
    local num2 : number
    local num3 : number
    local num4 : number
    local num5 : number
    local num6 : number
    local num7 : number
    local num8 : number

    str1, str2, str3 = strfn()
    int1, int2 = intfn()
    num1, num2, num3, num4, num5, num6, num7, num8 = numfn()

    write(str1, "\n", str2, "\n", str3, "\n", int1, "\n", int2,
    "\n", num1, "\n", num2, "\n", num3, "\n", num4, "\n", num5,
    "\n", num6, "\n", num7, "\n", num8, "\n")
end

main()

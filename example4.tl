require "ifj21"
function main()
    
    write("require \"ifj21\"\n")
    local i:integer = 0
    
    while i < 100 do
        write("function fun",i,"()","\n")

        write("end","\n\n")

        i = i + 1
    end

end

main()
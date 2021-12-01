require "ifj21"

function main()
    local x:number = 4.9
    local y:string = "hello there"

    -- try to compare number and string
    -- this should fail to compile
    if x == y then
        write("yes\n")
    else
        write("no\n")
    end
end

main()

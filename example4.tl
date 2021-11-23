require ""
global b : function(integer, string) : integer, integer, integer
function a ()
    local b : number = 8*"s"
    local a : integer
    a = 5
    b = 7
end

function b(a:integer, b:string) : integer, integer, integer
    a = 8

    if a == 7 then
        return 1+7, 7, 3
    end

    return 1+7, 7, 3
end


b(1, 8, 9)

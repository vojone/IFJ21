require ""
global b : function(integer, string) : integer, integer, integer
function a ()
    local b : number = 8*7
    local a : nil
end

function b(a:integer, b:string) : integer, integer, integer
    a = 8

    if a == 7 then
        return 1+7, 7, 3
    end

    return 1+7, 7, 3
end


b(1, 8, 9)

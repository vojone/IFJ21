function a(b:integer,c:integer)
    if b>=c then
        write("b>=c")
        if b==c then
            write("b==c")
        else
            write("b!=c")
        end
    else
        write("b<c")
    end
end
a(4,10)
function fdoesntworko() : string, string
    return "hehe"
end

function print(a : integer, b:integer)
    write(a,b)
end

function matrix(imax :integer, jmax:integer )
    local i : integer = 1
    local j : integer = 1
    while i <= imax do
        write("\n")
    
        while j <= jmax do
            print(i,j)
            
            write("\t")
            j = j + 1

        end
        i = i + 1
        j = 1

    end
end
matrix(3,4)
<<<<<<< Updated upstream
<<<<<<< Updated upstream
-- fdoesntworko()
=======
-- fdoesntworko()
>>>>>>> Stashed changes
=======
-- fdoesntworko()
>>>>>>> Stashed changes

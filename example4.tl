--jump %%ENDmain
--label %%FUNmain
function main (a : integer, b : integer) --jump
    write(a)

    if a == 0 then
        local a : number
        a = 0 - a
    end

    if a == 0 then
        local a : integer
    end
end
--label %%ENDmain


function mainB (a : integer, b : integer) --jump
    write(a)

    if a == 0 then
        local a : integer
        a = 0
    end

    if a == 0 then
        local a : integer
    end
end

main(10,4)


--[[
Symboltab0: "a" - data.name = main$a$1


Symboltab1: "a" - data.name = main$a$3

DEFVAR main$a$0
PUSH main$a$0
CALL write

...

DEFVAR main$a$1
... (expr)
POPS main$a$1


DEFVAR main$a$1
... (expr)
POPS main$a$1
]]

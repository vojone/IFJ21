--jump %%ENDmain
--label %%FUNmain
global foo : function(integer)

function main (a:integer,b:integer) --jump
    write(a)
    foo(a+b)
    write(b)
end

function heh(c: integer)
    write(c*4)
end

function foo (a:integer) --jump
    write(a)
    heh(a)
end
--label %%ENDmain
foo(3)
main(10,4)

--jump %%ENDmain
--label %%FUNmain
function main (a:integer,b:integer) --jump
    
    main(a+b,b)
end
--label %%ENDmain

main(10+2,4)

-- zkouška semantických edge cases
	-- teoreticky by to nemělo projít, protože se tu mixují datatypes, takovej guláš test, klidně ho vynech

		require "ifj21"
		global ndmain : function() : integer

		function main() 
			local x : integer
			x = ndmain()
			write("ndmain vrátí: ", x , "\n")

		end

		function ndmain() : integer
			local a : integer = 1
			local b : integer
			local c : string
			local d : integer
			local e : number

			b = 2
			d = 12
			e = 5
			c = "Hello World"
			d = a + (a + b * a) / a

			if d == a then
			write("d = ", d , "\n")
			else
			write("d = ", d , "\n")
			end

			return d
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Zkouška míchání datových typů -> number/integer
	-- Neprojde

	-- return SEMANTIC_ERROR_ASSIGNMENT;

		require "ifj21"
		function main3()
			local a : integer = 2
			local b : number = 10.5
			local c : integer = 0
			local d : number = 0

			c = a * b
			d = a * b
			write("Hodnota B je ", b ," a hodnota D je", d ,"\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Zkouška proměnné používání stejného ID proměnné ve více blocích
	-- Mělo by vypsat 3x hodnotu 'a', před, v a za blokem IF (1,3,1)
	-- Projde, ale záleží jak se zachová, protože ten write nefunguje s tím interpretem

	-- return 0;

		require "ifj21"
		function main4()
			local a : integer = 1
			local b : integer = 2
			write("A(1) před IF = ", a ,"\n")
			if b then
				a = 3
			write("A(3) v = ", a ,"\n")
			end
			write("A(1) za IF = ", a ,"\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Proměnná se jmenuje stejně jako funkce

	-- return SEMANTIC_ERROR_DEFINITION;

		require "ifj21"
		function main5()
			local main5 : string
			main5 = "testTextuProšel"
			write(main5, "\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Funkce se správným počtem a typem argumentů

	-- return 0;

		require "ifj21"
		function func(a : integer, b : integer) : integer
			a = b
			return a
		end

		function main6()
			local a : integer = 1
			local b : integer = 2
			func(a,b)
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Funkce se správným počtem a ale špatným typem argumentů

	-- return SEMANTIC_ERROR_PARAMETERS ; 

		require "ifj21"
		function func(a : integer, b : string) : integer
			b = "s"
			return a
		end

		function main7()
			local a : integer = 1
			local b : integer = 2
			func(a,b)
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Funkce se špatným počtem a typem argumentů
	-- return SEMANTIC_ERROR_PARAMETERS ;

		require "ifj21"
		function func(a : string, b : string, c : integer) : integer
			a = b
			return c
		end

		function main8()
			local a : integer = 1
			local b : integer = 2
			func(a,b)
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Testování returnu do více proměnných
	-- Vrátí String 'Hello World' a 1
	-- return 0;

		require "ifj21"
		function concatenate(c : string, d : string) : string, integer
			return c .. d, 1
		end

		function main9()
			local a : string
			local b : integer
			a,b = concatenate("Hello", "World")
			write(a , " " , b , "\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
 
	-- Testy na hodnotu . Not sure jestli to má projít ??

	-- return 0; nebo return UNEXPECTED_NIL_ERROR;

		require "ifj21"
		function main10()
			local a : integer = nil
			local b : string = nil
			local c : number = nil

			write("Hodnoty a,b,c jsou: ", a , b , c , "\n")
		end


-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
 
	-- Aritmetaka s hodnotou nil
	-- not sure bout this one 

	-- return 0; nebo return UNEXPECTED_NIL_ERROR;

		require "ifj21"
		function main11()
			local a : integer = 10
			local b : integer = nil

			a = a + b
			write("Hodnota a je: ", a , "\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Dělení 0
	-- return DIVISION_BY_ZERO_ERROR;

		require "ifj21"
		function main12()
			local a : integer = 10
			local c : integer
			c = a / 0
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Volání nedeklarované funkce
	-- Teoreticky by projít nemělo? Prakticky to interpretu nevadí :/

	-- return SEMANTIC_ERROR_DEFINITION;
		
		require "ifj21"
		function main13()
			foo()
		end

		function foo()
			local a : integer = 2
			write("Hodnota A je: ", a , "\n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
	-- Volání deklarované funkce
	-- Teoreticky by projít mělo?

	-- return 0;

		require "ifj21"
		global foo2 : function()
			
		function main14()
			foo2()
		end

		function foo2()
			local a : integer = 2
			write("Hodnota A je: ", a , "\n")
		end





-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
-- // -- // -- // -- // -- // -- // -- // -- // -- // -- //
-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
					SYNTAKTICKÉ TESTY
-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 
-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 


	-- Syntaktická chyba v deklaraci funkce

	-- return SYNTAX_ERROR;

		require "ifj21"
		functiom main15()
			write("Syntaktická chyba \n")
		end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba v IF bloku

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main16()
		local a : integer = 2
		if a == 2 them
			write("Syntaktická chyba \n")
		end
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, chybí 'end'

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main17()
		local a : integer = 3
		if a == 3 then
			write("Syntaktická chyba \n")
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, závorky u definice funkce

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main18

	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, závorky u volání funkce

	-- return SYNTAX_ERROR;

	require "ifj21"

	function zavorky()

	end

	function main19()
		zavorky
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, překlep v IF

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main20()
		local a : integer = 2
		iff a == 2 then

		end
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, rovnítko ve výrazu iffu

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main21()
		local a : integer = 2
		if a = 2 then

		end
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, chyba ve while bloku

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main22()
		local a : integer = 2

		while a > 0 d00

		end
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Lexikální chyba, neočekávaný znak ';'

	-- return LEXICAL_ERROR;

	require "ifj21"
	function main23();

	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Lexikální chyba, neočekávaný znak ';'

	-- return LEXICAL_ERROR;

	require "ifj21"
	;function main24()

	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Lexikální chyba, neočekávaný znak

	-- return LEXICAL_ERROR;

	require "ifj21"
	function main25()

	end;



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, neočekávané rovnítko

	-- return SYNTAX_ERROR;

	require "ifj21"
	function main26() =

	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, neočekávaný znak ':'

	-- return SYNTAX;

	require "ifj21"
	function main27()
		local a : string
	end



-- // -- // -- // -- // -- // -- // -- // -- // -- // -- // 

	-- Syntaktická chyba, chybějící then

	-- return SYNTAX_ERR;

	require "ifj21"
	function main28()
		local a : integer = 2
		if a = 2

		end
	end

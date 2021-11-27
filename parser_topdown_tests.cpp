/******************************************************************************
 *                                  IFJ21
 *                               pp_tests.cpp
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *                 Purpose: Unit tests for precedence parser
 * 
 *                      Last change: 30. 10. 2021
 *****************************************************************************/ 

/**
 * @file parser_test.c
 * @brief Unit tests for recursive parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#define TMP_FILE_NAME "tmp.inp" /**< Temporary file for testing inputs (stream from this file is redirected to stdin)*/

bool verbose_mode = false; /**< Tests prints only test cases with differences between expected result and got */
bool force_mode = false; /**< Tests automatically prevents overwriting files, if is set to false */

extern "C" {
	#include "parser_topdown.h"
}

#include "gtest/gtest.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

FILE * create_input_file(std::string *name, std::string *content) {
	if(!force_mode) {
		FILE * test_file = fopen(name->c_str(), "r");
		if(test_file) {
			fprintf(stderr, "Parser test:");
			fprintf(stderr, "Err: File %s exists!\n", name->c_str());
			fclose(test_file);

			return NULL;
		}
	}

	FILE * input_file = fopen(name->c_str(), "w");
	if(!input_file) {
		fprintf(stderr, "Parser test: Err: Can't create input file!");

		return NULL;
	}
	
	//printf("%s\n", content->c_str());
	fprintf(input_file, "%s", content->c_str());

	return input_file;
}

bool prepare_tests(std::string *fname, std::string *content, scanner_t *dut, parser_t *pt) {
	FILE * input = NULL;
	input = create_input_file(fname, content);
	if(!input) {
		return false;
	}

	fclose(input);
	freopen(fname->c_str(), "r", stdin);

	scanner_init(dut);
	parser_setup(pt, dut);

	return true;
}

class test_fixture : public :: testing :: Test {
	protected:
		std::string inp_filename = TMP_FILE_NAME;
		std::string scanner_input;

		scanner_t uut;
		parser_t pt;
		bool init_success;

		virtual void setData() {
			scanner_input = R"()";
		}


		virtual void SetUp() {
			setData();
			init_success = prepare_tests(&inp_filename, &scanner_input, &uut, &pt);
			if(!init_success) {
				scanner_dtor(&uut);
				exit(EXIT_FAILURE);
			}
		}

		virtual void TearDown() {
			remove(inp_filename.c_str());
			scanner_dtor(&uut);
		}
};

TEST_F(test_fixture, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class an_error : public test_fixture {
	protected:
		void setData() override {
			//not 100% sure wheter this should be SYNTAX_ERROR or LEXICAL_ERROR
			scanner_input = 
			R"( function main() local ; : integer end
			)";
		}
};

TEST_F(an_error, only_parse) {
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class basic : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(require "help.tl" function main() local s1 : integer local s2 : string = ", ktery jeste trochu obohatime" end
			)";
		}
};

TEST_F(basic, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class if_parse : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(require "help.tl" 
			function main() 
				local s1 : integer 
				if s1 == 0 then 
					local s2 : string = ", ktery jeste trochu obohatime" 
				else
					s1 = 0
				end
			end
			)";
		}
};

TEST_F(if_parse, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class if_parse_err1 : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(require "help.tl" 
			function main() 
				local s1 : integer 
				if true then 
					local s2 : string = ", ktery jeste trochu obohatime" 
				else
					s2 = 0
				end
			end
			)";
		}
};

TEST_F(if_parse_err1, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class function_parse_err : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(require "help.tl" 

			function abc()
				local s1 : integer
			end

			function main()
				s1 = 0
			end
			)";
		}
};

TEST_F(function_parse_err, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}


class if_parse_not_err : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(require "help.tl" 
			function main() 
				local s1 : integer 
				if s1 then 
					local s1 : string = ", ktery jeste trochu obohatime" 
				else
					s1 = 0
				end
			end
			)";
		}
};

TEST_F(if_parse_not_err, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class while_parse : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main() 
					local s1 : integer 
					local s2 : number

					s1, s2 = 3, 4
					while s1 do
						s1 = 2
					end
				end
			)";
		}
};

TEST_F(while_parse, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class parameters : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a : string, c: number)
					local s1 : string
				end
			)";
		}
};

TEST_F(parameters, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class function_types : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main() : string, number
					local s1 : string

					return 1, "h"
				end
			)";
		}
};

TEST_F(function_types, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_PARAMETERS);
}


class function_return : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main() : string
					local s1 : string = "As I Was"
					if c then
						return a
					else 
						return s1
					end
				end
			)";
		}
};

TEST_F(function_return, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}


class function_return2 : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a : string, c: integer) : string, integer
					local s1 : string = "As I Was"
					if c then
						return a, 2
					else 
						return s1, 0
					end
				end
			)";
		}
};

TEST_F(function_return2, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class function_return3 : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a : string, c: integer) : string, integer
					local s1 : string = "As I Was"
					if c then
						return a, 2
					else 
						
					end
				end
			)";
		}
};

TEST_F(function_return3, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}



class function_return4 : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a : string, c: integer) : string, integer
					local s1 : string = "As I Was"
					if c then
						return a, 2
					else 
						
					end
				end
			)";
		}
};

TEST_F(function_return4, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}



class missing_end : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a : string, c: integer) : string, integer
					if c then
						return a, 2
					//missing end
				end
			)";
		}
};

TEST_F(missing_end, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}


class multiple_assignment : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main(a: string, c: integer) : string, integer
					local s1 : string = "As I Was"
					local b : number
					s1, b = a, c

					return "a"
				end
			)";
		}
};

TEST_F(multiple_assignment, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class string_expect : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				require //missing file string
				function main()
				end
			)";
		}
};

TEST_F(string_expect, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class function_call : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(
				function main() : string, integer
				end
				main("a","string")
			)";
		}
};

TEST_F(function_call, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_OTHER);
}

class double_assignment_function : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main(a : string, c: integer) : string, integer
					local s1 : string = "As I Was"
					local b : integer
					if a then    
						s1, b = main(a,c)
					end

					return "", 0
				end
			)";
		}
};

TEST_F(double_assignment_function, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class wrong_declaration : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					local s1 : string = "As I Was"
					b : boolean
				end
			)";
		}
};

TEST_F(wrong_declaration, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class expression_missing : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					if then
					end
				end
			)";
		}
};

TEST_F(expression_missing, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}


class wrong_assignment : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					local s1 : string = "As I Was"
					s1, "aa"  = "bb", 2
				end
			)";
		}
};

TEST_F(wrong_assignment, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}


class missing_comma : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					local s1 : string = "As I Was"
					local s2 : string = "As I Was"
					s1 s2  = "bb", 2
				end
			)";
		}
};

TEST_F(missing_comma, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class missing_bracket : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					write ) 
				end
			)";
		}
};

TEST_F(missing_bracket, only_parse) {
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}


class function_declaration : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( global a : function (number) : number
				function a (b : number) : number
					return 10.1
				end
			)";
		}
};

TEST_F(function_declaration, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class complexity_moderate : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(-- Program 3: Prace s retezci a vestavenymi funkcemi
				require "ifj21"
				function main()
				local s1 : string = "Toto je nejaky text"
				local s2 : string = s1 .. ", ktery jeste trochu obohatime"
				write(s1, "\010", s2)local s1len:integer=#s1 local s1len4: integer=s1len
				s1len = s1len - 4 s1 = substr(s2, s1len, s1len4) 
				s1len = s1len + 1
				write("4 znaky od", s1len, ". znaku v \"", s2, "\":", s1, "\n")
				write("Zadejte serazenou posloupnost vsech malych pismen a-h, ")
				write("pricemz se pismena nesmeji v posloupnosti opakovat: ")
				s1 = reads()
				if s1 ~= nil then
				while s1 ~= "abcdefgh" do
				write("\n", "Spatne zadana posloupnost, zkuste znovu:")
				s1 = reads()
				end
				else
				end
				end
				main()
			)";
		}
};

TEST_F(complexity_moderate, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class complexity_factorial : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(-- Program 2: Vypocet faktorialu (rekurzivne)
require "ifj21"
function factorial(n : integer) : integer
local n1 : integer = n - 1
if n < 2 then
return 1
else
local tmp : integer = factorial(n1)
return n * tmp
end
end
function main()
write("Zadejte cislo pro vypocet faktorialu: ")
local a : integer = readi()
if a ~= nil then
if a < 0 then
write("Faktorial nejde spocitat!", "\n")
else
local vysl : integer = factorial(a)
write("Vysledek je ", vysl, "\n")
end
else
write("Chyba pri nacitani celeho cisla!\n")
end
end
main()
			)";
		}
};

TEST_F(complexity_factorial, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class complexity_factorial2 : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"(-- Program 1: Vypocet faktorialu (iterativne)
require "ifj21"
function main() -- uzivatelska funkce bez parametru
local a : integer
local vysl : integer = 0
write("Zadejte cislo pro vypocet faktorialu\n")
a = readi()
if a == nil then
write("a je nil\n") return
else
end
if a < 0 then
write("Faktorial nelze spocitat\n")
else
vysl = 1
while a > 0 do
vysl = vysl * a a = a - 1 -- dva prikazy
end
write("Vysledek je: ", vysl, "\n")
end
end
main() -- prikaz hlavniho tela programu
			)";
		}
};

TEST_F(complexity_factorial2, only_parse) {
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class function_check_global : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
				end
				mainN(true,1,0)
			)";
		}
};

TEST_F(function_check_global, sema) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class function_check_inside : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
				end
				function hello()
					mainN(true,1,0)
				end
			)";
		}
};

TEST_F(function_check_inside, only_parse) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class variable_check_local : public test_fixture {
	protected:
		void setData() override {
			scanner_input = 
			R"( function main()
					local a : number
					a = "Lorem ipsum dolor sit amet"
				end
			)";
		}
};

TEST_F(variable_check_local, semantic) {
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class hash_check_1 : public test_fixture{
	protected: 
		void setData() override{
			scanner_input =
			R"(
				-- Funkce vyzkouší operace '#'

				require "ifj21"
				function main()
					local a : string = "World Hello?"
					local b : integer

					b = #a
					write("Delka řetězce a je", b , "\n")
				end
			)";
		}
};

TEST_F(hash_check_1, semantic){
	ASSERT_EQ(parse_program(),PARSE_SUCCESS);
}

class hash_check_2 : public test_fixture{
	protected: 
		void setData() override{
			scanner_input =
			R"(
				-- Funkce vyzkouší operace '#'

				require "ifj21"
				function main()
					local a : integer

					a = #"Test?"
					write("Delka řetězce je", a , "\n")
				end
			)";
		}
};

TEST_F(hash_check_2, semantic){
	ASSERT_EQ(parse_program(),PARSE_SUCCESS);
}

class multiple_commands: public test_fixture{
	protected: 
		void setData() override{
			scanner_input =
			R"(
				-- Test více příkazů na jednom řádku

				require "ifj21"
				function main()
					local a: integer = 2 local b : number = 3 local c :string ="LMFAO" a= #c

				end
			)";
		}
};

TEST_F(multiple_commands, semantic){
	ASSERT_EQ(parse_program(),PARSE_SUCCESS);
}

class mixing_data_types_1 : public test_fixture{
	protected:
		void setData() override {
			scanner_input =
			R"(
				-- Zkouška míchání datových typů NUM+INT => INT
				-- Podle zadání 
				-- "Jsou-li oba operandy typu number nebo
				-- jeden integer a druhý number, výsledek je typu number."
				-- Takže nevím, jestli se c automaticky přetypuje, nebo ne, opravím později

				require "ifj21"
				function main()
					local a : integer = 2
					local b : number = 10.5
					local c : integer = 0

					c = a + b
					write("Hodnota B je ", b ,"\n")
				end
			)";
		}
};

TEST_F(mixing_data_types_1, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_ASSIGNMENT);
}

class mixing_data_types_2 : public test_fixture{
	protected:
		void setData() override {
			scanner_input =
			R"(
				-- Zkouška míchání datových typů NUM+INT => NUM
				-- Projde

				require "ifj21"
				function main()
					local a : integer = 2
					local b : number = 10.5
					local c : number

					c = a + b
					write("Hodnota B je ", b ,"\n")
				end
			)";
		}
};

TEST_F(mixing_data_types_2, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class same_variable_different_blocks : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Použití stejné proměnné ve více blocích

				require "ifj21"
				function main()
					local a : integer = 1
					local b : integer = 2
					write("A(1) před IF = ", a ,"\n")
					if b then
						a = 3
					write("A(3) v = ", a ,"\n")
					end
					write("A(1) za IF = ", a ,"\n")
				end
			)";
		}
};

TEST_F(same_variable_different_blocks, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class variable_as_function : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Proměnná se jmenuje stejně jako funkce

				require "ifj21"
				function main()
					local main : string
					main = "testTextuProšel"
					write(main, "\n")
				end
			)";
		}
};

TEST_F(variable_as_function, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}


class variable_as_function2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Proměnná se jmenuje stejně jako funkce

				require "ifj21"
                function a()
                    write("Vojta was here")
                end

				function main()
					local a : string
					a = "testTextuProšel"
					write(a, "\n")
				end
			)";
		}
};

TEST_F(variable_as_function2, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class function_as_variable : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Funkce se jmenuje stejně jako proměnná

				require "ifj21"
				function main()
					local lmao : string
					lmao = "testTextuProšel"
					lmao()
					write(lmao, "\n")
				end

				function lmao()

				end
			)";
		}
};

TEST_F(function_as_variable, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class return_statement : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Funkce se správným počtem a typem argumentů

				require "ifj21"
				function func(a : integer, b : integer) : integer
					a = b
					return a
				end

				function main()
					local a : integer = 1
					local b : integer = 2
					func(a,b)
				end
			)";
		}
};

TEST_F(return_statement, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class return_statement_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Funkce se správným počtem a ale špatným typem argumentů

				require "ifj21"
				function func(a : integer, b : string) : integer
					b = "s"
					return a
				end

				function main()
					local a : integer = 1
					local b : integer = 2
					func(a,b)
				end
			)";
		}
};

TEST_F(return_statement_2, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_PARAMETERS);
}

class return_statement_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Funkce se špatným počtem a typem argumentů

				require "ifj21"
				function func(a : string, b : string, c : integer) : integer
					a = b
					return c
				end

				function main()
					local a : integer = 1
					local b : integer = 2
					func(a,b)
				end
			)";
		}
};

TEST_F(return_statement_3, semanic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_PARAMETERS);
}

class return_statement_4 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Funkce s více hodnotama

				require "ifj21"
				function func(a : integer, b : integer) : integer, integer
					return a, b
				end

				function main()
					local a : integer = 1
					local b : integer = 3
					local c : integer
					local d : integer
					c, d = func(a,b)
					write("Hodnota a: ", a ,", hodnota b: ", b ,"\n")
				end
			)";
		}
};

TEST_F(return_statement_4, semanic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class return_statement_5 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Funkce s předanou hodnotou nil

				require "ifj21"
				function func(a : integer, b : integer) : integer, integer
					return a, b
				end

				function main()
					local a : integer = 1
					local b : integer = nil
					local c : integer
					local d : integer
					c, d = func(a,b)
					write("Hodnota a: ", a ,", hodnota b: ", b ,"\n")
				end
			)";
		}
};

TEST_F(return_statement_5, semanic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class return_to_var : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Testování returnu do více proměnných
				-- Vrátí String 'Hello World' a 1
				-- return 0;

				require "ifj21"
				function concatenate(c : string, d : string) : string, integer
					return c .. d, 1
				end

				function main()
					local a : string
					local b : integer
					a,b = concatenate("Hello", "World")
					write(a , " " , b , "\n")
				end
			)";
		}
};

TEST_F(return_to_var, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_assignment : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil
				-- Not sure jestli to má projít ??

				require "ifj21"
				function main()
					local a : integer = nil
					local b : string = nil
					local c : number = nil

					write("Hodnoty a,b,c jsou: ", a, b, c ,"\n")
				end
			)";
		}
};

TEST_F(nil_assignment, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_in_function : public test_fixture{ //????
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil ve funkci
				-- Nemá projít, protože se předává nil
				-- Aktuálně to vrací hodnotu 7 (SEMANTIC_ERROR_OTHER)
				-- ale má vracet 8 (UNEXPECTED_NIL_ERROR)

				function chr(i : integer) : string
					local c : string = "s"
					return c
				end

				require "ifj21"
				function main()
					local a : integer = nil
					local b : string
					b = chr(a)
					write("Hodnoty a,b jsou: ", a, b,"\n")
				end

			)";
		}
};

TEST_F(nil_in_function, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_OTHER);
}

class nil_in_relations_1 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil v relaci

				require "ifj21"
				function main()
					local a : integer = nil
					local b : integer = 2
					if a > b then

					end
				end

			)";
		}
};

TEST_F(nil_in_relations_1, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_in_relations_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil v relaci

				require "ifj21"
				function main()
					local a : integer = nil
					local b : integer = 2
					if a <= b then
						
					end
				end

			)";
		}
};

TEST_F(nil_in_relations_2, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_in_relations_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil v relaci

				require "ifj21"
				function main()
					local a : integer = nil
					local b : integer = 2
					if a == b then
						
					end
				end

			)";
		}
};

TEST_F(nil_in_relations_3, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_in_relations_4 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Testy na hodnotu nil v relaci

				require "ifj21"
				function main()
					local a : integer = nil
					local b : integer = 2
					if a ~= b then
						
					end
				end

			)";
		}
};

TEST_F(nil_in_relations_4, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_arithmetics : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Aritmetika s hodnotou nil
				-- not sure bout this one, ale nejspíš chyba

				require "ifj21"
				function main()
					local a : integer = 10
					local b : integer = nil

					a = a + b
					write("Hodnota a je: ", a , "\n")
				end
			)";
		}
};

TEST_F(nil_arithmetics, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class nil_arithmetics_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Aritmetika s hodnotou nil
				-- 

				require "ifj21"
				function main()
					local a : integer = 10
					local b : string = nil

					a = a + b
					write("Hodnota a je: ", a , "\n")
				end
			)";
		}
};

TEST_F(nil_arithmetics_2, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_EXPRESSION);
}

class nil_arithmetics_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Aritmetaka s hodnotou nil
				-- not sure bout this one, ale nejspíš chyba

				require "ifj21"
				function main()
					local a : integer = nil
					local b : integer = 2

					a = a - b
					write("Hodnota a je: ", a , "\n")
				end
			)";
		}
};

TEST_F(nil_arithmetics_3, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class division_by_zero_1 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Dělení 0

				require "ifj21"
				function main()
					local a : integer = 10
					local c : integer
					c = a / 0
				end
			)";
		}
};

TEST_F(division_by_zero_1, semantic){
	ASSERT_EQ(parse_program(), DIVISION_BY_ZERO_ERROR);
}

class division_by_zero_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Dělení 0

				require "ifj21"
				function main()
					local a : integer = 10
					local c : integer
					c = a // 0
				end
			)";
		}
};

TEST_F(division_by_zero_2, semantic){
	ASSERT_EQ(parse_program(), DIVISION_BY_ZERO_ERROR);
}

class undeclared_function : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Volání nedeklarované funkce
				-- Teoreticky by projít nemělo? Prakticky to interpretu nevadí :/

				require "ifj21"
				function main()
					foo()
				end

				function foo()
					local a : integer = 2
					write("Hodnota A je: ", a , "\n")
				end
			)";
		}
};

TEST_F(undeclared_function, semantic){
	ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
}

class declared_function : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Volání deklarované funkce
				-- Teoreticky by projít mělo?

				-- return 0;

				require "ifj21"
				global foo2 : function()
					
				function main()
					foo2()
				end

				function foo2()
					local a : integer = 2
					write("Hodnota A je: ", a , "\n")
				end
			)";
		}
};

TEST_F(declared_function, semantic){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class function_declaration_sx : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba v deklaraci funkce
				-- Jak jsem to nechal projít, tak si actually nejsem jistej proč to
				-- hází semantic error, monžná to bere jako identifier ?
				-- Will clarify

				-- return SYNTAX_ERROR;

				require "ifj21"

				functiom main()
					write("Syntaktická chyba \n")
				end
			)";
		}
};

TEST_F(function_declaration_sx, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class if_statement_syntax : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba v IF bloku

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 2
					if a == 2 them
						write("Syntaktická chyba \n")
					end
				end
			)";
		}
};

TEST_F(if_statement_syntax, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class if_statement_syntax_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, překlep v IF

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 2
					iff a == 2 then

					end
				end
			)";
		}
};

TEST_F(if_statement_syntax_2, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class if_statement_syntax_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, rovnítko ve výrazu IF

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 2
					if a = 2 then

					end
				end
			)";
		}
};

TEST_F(if_statement_syntax_3, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class if_statement_syntax_4 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Syntaktická chyba, chybějící then

				-- return SYNTAX_ERR;

				require "ifj21"
				function main28()
					local a : integer = 2
					if a = 2

					end
				end
			)";
		}
};

TEST_F(if_statement_syntax_4, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class missing_end_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, chybí 'end'

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 3
					if a == 3 then
						write("Syntaktická chyba \n")
				end
			)";
		}
};

TEST_F(missing_end_2, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class parentheses_function_def : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, závorky u definice funkce

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main

				end
			)";
		}
};

TEST_F(parentheses_function_def, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class parentheses_function_call : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, závorky u volání funkce

				-- return SYNTAX_ERROR;

				require "ifj21"

				function zavorky()

				end

				function main()
					zavorky
				end
			)";
		}
};

TEST_F(parentheses_function_call, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class parentheses_too_many : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Hodně závorek, projde (SUPR!!!)

				require "ifj21"
				function main()
				local b : integer = 2
				local a : integer = ( ((b) + (b)) * ( (b)- (b) + (b) ) - ( (b+b)-(b)*(b)) ) + #"TŘI"
				end
			)";
		}
};

TEST_F(parentheses_too_many, syntax){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}

class unexpected_char : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak ';'

				-- return LEXICAL_ERROR;

				require "ifj21"
				function main();

				end
			)";
		}
};

TEST_F(unexpected_char, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '='

				-- return LEXICAL_ERROR;

				= require "ifj21"
				function main()

				end
			)";
		}
};

TEST_F(unexpected_char_2, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '~'
				-- Toto je hrozně weirdChamp, ono to bere tu tilde jako název funkce ._. (moje chyba ve scanneru - Vojta)

				-- return LEXICAL_ERROR;

				require "ifj21"
				function ~main()

				end
			)";
		}
};

TEST_F(unexpected_char_3, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_4 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '´'

				-- return LEXICAL_ERROR;

				require "ifj21"´
				function main()

				end
			)";
		}
};

TEST_F(unexpected_char_4, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_5 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak ';'

				-- return LEXICAL_ERROR;

				require "ifj21"
				function main()

				end;
			)";
		}
};

TEST_F(unexpected_char_5, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_6 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '/'
				-- Podle mě by mělo hodit lexical, will clarify

				-- return LEXICAL_ERROR;

				require "ifj21"
				function main()
				/
				end
			)";
		}
};

TEST_F(unexpected_char_6, lexical){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class unexpected_char_7 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Syntaktická chyba, neočekávaný znak ':'

				-- return SYNTAX;

				require "ifj21"
				function main27()
					local a : : string
				end
			)";
		}
};

TEST_F(unexpected_char_7, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class unexpected_char_8 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '#'

				-- return SYNTAX;

				#
				require "ifj21"
				function main27()
					local a : string
				end
			)";
		}
};

TEST_F(unexpected_char_8, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

class unexpected_char_9 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak za main() '('

				-- return SYNTAX;

				require "ifj21"
				function main27()(
					local a : string
				end
			)";
		}
};

TEST_F(unexpected_char_9, lexical){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class unexpected_char_10 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Syntaktická chyba, neočekávaný znak main( '(' )

				-- return SYNTAX;

				require "ifj21"
				function main27(()
					local a : string
				end
			)";
		}
};

TEST_F(unexpected_char_10, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class unexpected_char_11 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, špatné závorky v IF

				-- return SYNTAX;

				require "ifj21"
				function main27()
					local a : string = "s"
					if [a = "s"] then

					end
				end
			)";
		}
};

class unexpected_char_12 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- Lexikální chyba, neočekávaný znak '!'

				-- return LEXICAL_ERROR;

				require "ifj21"
				function !main()

				end
			)";
		}
};

TEST_F(unexpected_char_12, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}

TEST_F(unexpected_char_11, syntax){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}


class unexpected_char_13 : public test_fixture{
	protected:
		void setData() override{
			scanner_input = 
			R"(
				-- return LEXICAL_ERROR;

				require "ifj21"
				function main()
                    local a : number = 123efg
				end
			)";
		}
};

TEST_F(unexpected_char_13, lexical){
	ASSERT_EQ(parse_program(), LEXICAL_ERROR);
}


class while_statement_syntax_1 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, chyba ve while bloku

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 2

					while a > 0 d00

					end
				end
			)";
		}
};

TEST_F(while_statement_syntax_1, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class while_statement_syntax_2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Syntaktická chyba, chyba ve while bloku

				-- return SYNTAX_ERROR;

				require "ifj21"
				function main()
					local a : integer = 2

					whilst a != 0 do

					end
				end
			)";
		}
};

TEST_F(while_statement_syntax_2, syntax){
	ASSERT_EQ(parse_program(), SYNTAX_ERROR);
}

class while_statement_syntax_3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(

				-- return PARSE_SUCCESS;

				require "ifj21"
				function main()
					local a : integer = 2

					while a > 0 do
						a = a - 1
					end
				end
			)";
		}
};

TEST_F(while_statement_syntax_3, syntax){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class coment_test1 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Komentáře

				require "ifj21"
				function main()
					-- world says 
					--[[hello there]]
				end
			)";
		}
};

TEST_F(coment_test1, syntax){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


class coment_test2 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Komentáře

				require "ifj21"
				--[[function main()
					-- world says 
					--[[hello there
				end
			)";
		}
};

TEST_F(coment_test2, syntax){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}



class coment_test3 : public test_fixture{
	protected:
		void setData() override{
			scanner_input =
			R"(
				-- Komentáře

				---- -require "ifj21"
				---function main()
					-- world says 
					--[[hello there]]
				--[[]end]-]
			)";
		}
};

TEST_F(coment_test3, syntax){
	ASSERT_EQ(parse_program(), PARSE_SUCCESS);
}


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	for(int i = 0; i < argc; i++) {
		if(strcmp(argv[i], "-f") == 0) {
			force_mode = true;
		}
		else if(strcmp(argv[i], "-v") == 0) {
			verbose_mode = true;
		}
	}

	return RUN_ALL_TESTS();
}


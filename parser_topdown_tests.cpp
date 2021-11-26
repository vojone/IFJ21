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
    ASSERT_EQ(parse_program(), SEMANTIC_ERROR_DEFINITION);
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

                    return "a", 8
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


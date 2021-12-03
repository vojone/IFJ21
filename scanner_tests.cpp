/******************************************************************************
 *                                  IFJ21
 *                             scanner_test.cpp
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *                 Purpose: Unit tests for scanner
 * 
 *                      Last change: 30. 10. 2021
 *****************************************************************************/ 

/**
 * @file scanner_test.c
 * @brief Unit tests for scanner
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#define TMP_FILE_NAME "tmp.inp" /**< Temporary file for testing inputs (stream from this file is redirected to stdin)*/

extern "C" {
    #include "scanner.h"
}

#include "gtest/gtest.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

bool verbose_mode = false; /**< Tests prints only test cases with differences between expected result and got */
bool force_mode = false; /**< Tests automatically prevents overwriting files, if is set to false */

FILE * create_input_file(std::string *name, std::string *content) {
    if(!force_mode) {
        FILE * test_file = fopen(name->c_str(), "r");
        if(test_file) {
            fprintf(stderr, "Scanner test:");
            fprintf(stderr, "Err: File %s exists!\n", name->c_str());
            fclose(test_file);

            return NULL;
        }
    }

    FILE * input_file = fopen(name->c_str(), "w");
    if(!input_file) {
        fprintf(stderr, "Scanner test: Err: Can't create input file!");

        return NULL;
    }
    
    //printf("%s\n", content->c_str());
    fprintf(input_file, "%s", content->c_str());

    return input_file;
}

bool prepare_tests(std::string *fname, std::string *content, scanner_t *dut) {
    FILE * input = NULL;
    input = create_input_file(fname, content);
    if(!input) {
        return false;
    }

    fclose(input);
    freopen(fname->c_str(), "r", stdin);

    scanner_init(dut);

    return true;
}

void show_buffer(scanner_t *uut) {
    for(size_t i = 0; i <= uut->str_buffer.length; i++) {
        fprintf(stderr, "%d|", uut->str_buffer.str[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Length: %ld ( + \\0 at the end)", uut->str_buffer.length);
    fprintf(stderr, "\n");    
}
 
class test_fixture : public ::testing::Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input;
        std::vector<token_type_t> exp_types;
        std::vector<std::string> exp_attrs; 

        scanner_t uut;
        bool init_success;

        virtual void setData() {
            scanner_input = "";
            exp_types = {EOF_TYPE};
        }

        virtual void testTypes() {
            token_t temp;

            if(verbose_mode) {
                printf("\nI\tGot.\tExp.\tPos.\tAttr.\n");
            }
            for(size_t i = 0; i < exp_types.size(); i++) {
                temp = get_next_token(&uut);
            
                if(verbose_mode) {
                    printf("[%ld]\t%d\t%d", i, temp.token_type, exp_types[i]);
                    //printf("\t%lu %lu", uut.cursor_pos[ROW], uut.cursor_pos[COL]);
                    printf("\t%s", (char *)get_attr(&temp, &uut));
                    printf("\n");
                }

                ASSERT_EQ(exp_types[i], temp.token_type);
            }
        }

        virtual void testAttributes() {
            token_t temp;

            if(verbose_mode) {
                printf("\nI\tGot.\tExp.\tPos.\tAttr.\n");
            }
            for(size_t i = 0; i < exp_attrs.size(); i++) {
                temp = get_next_token(&uut);
            
                if(verbose_mode) {
                    printf("[%ld]\t%d\t%s", i, temp.token_type, exp_attrs[i].c_str());
                    //printf("\t%lu %lu", uut.cursor_pos[ROW], uut.cursor_pos[COL]);
                    printf("\t%s", (char *)temp.attr);
                    printf("\n");
                }

                std::string tmp_str((char *)get_attr(&temp, &uut));
                ASSERT_EQ(exp_attrs[i], tmp_str);
            }
        }

        virtual void SetUp() {
            setData(); 
            init_success = prepare_tests(&inp_filename, &scanner_input, &uut);
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

TEST_F(test_fixture, types) {
    testTypes();
}


class random_tokens : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(abc hi 45 1231.456 12311e2 ( ) "s" ===- * + <= >= .. 
            abcdefghchijklmnopqrstu // /
            #"length" "Ahoj\n\"Sve'te \\\034" 100e+10 10E-5)";
            exp_types = {IDENTIFIER, IDENTIFIER, INTEGER, 
                         NUMBER, NUMBER, SEPARATOR, 
                         SEPARATOR, STRING, OPERATOR, OPERATOR, 
                         OPERATOR, OPERATOR, OPERATOR, OPERATOR, 
                         OPERATOR, OPERATOR, IDENTIFIER, 
                         OPERATOR, OPERATOR, OPERATOR, STRING, 
                         STRING, NUMBER, NUMBER, EOF_TYPE};

            exp_attrs = {"abc", "hi", "45", "1231.456", "12311e2", "(", ")",
                         "\"s\"", "==", "=", "-", "*", "+", "<=", ">=", "..",
                         "abcdefghchijklmnopqrstu", "//", "/", "#", "\"length\"",
                         "\"Ahoj\\n\\\"Sve'te \\\\\\034\"", "100e+10", "10E-5"};
        };
};

TEST_F(random_tokens, types) {
    testTypes();
}


TEST_F(random_tokens, attributes) {
    testAttributes();
}


class whitespaces : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(--[[]]
            
            --dzktdtzdztktdjjtdz
            ---
            a
            ---[[safddasfdfs
            asdffdsa
            adsfdsaf
            dsaffdsa
            
            ]]
            b

            )";

            exp_types = {IDENTIFIER, IDENTIFIER, EOF_TYPE};
        }
};


TEST_F(whitespaces, types) {
    testTypes();
}



class lexical_errors : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(  ; ; ; ; === -200end"
            )";
            exp_types = {ERROR_TYPE, ERROR_TYPE, ERROR_TYPE, ERROR_TYPE,
                        OPERATOR, OPERATOR, OPERATOR, ERROR_TYPE, IDENTIFIER, ERROR_TYPE,
                        EOF_TYPE};

            exp_attrs = {";", ";", ";", ";", "==", "=", "-", "200e", "nd"};
        }
};


TEST_F(lexical_errors, types) {
    testTypes();
}

TEST_F(lexical_errors, attributes) {
    testAttributes();
}


class code_sample1 : public test_fixture {
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
                    write("Vysledek je: ", vysl,"\n")
                end
            end)";

            exp_types = {
            KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR, 
            SEPARATOR, KEYWORD,  IDENTIFIER, SEPARATOR, KEYWORD,
            KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD, OPERATOR, 
            INTEGER, IDENTIFIER, SEPARATOR, STRING, SEPARATOR, 
            IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, SEPARATOR, 
            KEYWORD, IDENTIFIER, OPERATOR, KEYWORD, KEYWORD, 
            IDENTIFIER, SEPARATOR, STRING, SEPARATOR, KEYWORD, 
            KEYWORD, KEYWORD,KEYWORD, IDENTIFIER, OPERATOR, 
            INTEGER, KEYWORD, IDENTIFIER,SEPARATOR, STRING,
            SEPARATOR, KEYWORD, IDENTIFIER, OPERATOR,INTEGER, 
            KEYWORD, IDENTIFIER, OPERATOR, INTEGER, KEYWORD,
            IDENTIFIER, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, 
            IDENTIFIER,OPERATOR, IDENTIFIER, OPERATOR, INTEGER, 
            KEYWORD, IDENTIFIER,SEPARATOR, STRING, SEPARATOR, 
            IDENTIFIER, SEPARATOR, STRING, SEPARATOR, KEYWORD, 
            KEYWORD, EOF_TYPE};
        }
};


TEST_F(code_sample1, types) {
    testTypes();
}


class code_sample2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            require "ifj21"

            global g : function (integer) : integer -- deklarace funkce
            function f(x : integer) : integer
                if x > 0then return x-1
                else
                    write("calling g with ", x)
                return g(x)
                end
            end

            function g(x : integer) : integer
                if x > 0 then
                    write("calling f with ", x)
                    return f(x)
                else return -200 end -- proč musí být před end bílý znak?
            end

            function main()
                local res : integer = g(10)
                write(res)
            end main()
            )";

            exp_types = {
                KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR,
                KEYWORD, SEPARATOR, KEYWORD, SEPARATOR, SEPARATOR, 
                KEYWORD, KEYWORD, IDENTIFIER, SEPARATOR, IDENTIFIER,
                SEPARATOR, KEYWORD, SEPARATOR, SEPARATOR, KEYWORD,
                KEYWORD, IDENTIFIER, OPERATOR, INTEGER, KEYWORD, 
                KEYWORD, IDENTIFIER, OPERATOR, INTEGER, KEYWORD, IDENTIFIER, 
                SEPARATOR, STRING, SEPARATOR, IDENTIFIER, SEPARATOR, 
                KEYWORD, IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, 
                KEYWORD, KEYWORD, KEYWORD, IDENTIFIER, SEPARATOR, 
                IDENTIFIER, SEPARATOR, KEYWORD, SEPARATOR, SEPARATOR, 
                KEYWORD, KEYWORD, IDENTIFIER, OPERATOR, INTEGER, 
                KEYWORD, IDENTIFIER, SEPARATOR, STRING, SEPARATOR, 
                IDENTIFIER, SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, 
                IDENTIFIER, SEPARATOR, KEYWORD, KEYWORD, OPERATOR, INTEGER, 
                KEYWORD, KEYWORD, KEYWORD, IDENTIFIER, SEPARATOR, 
                SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD, 
                OPERATOR, IDENTIFIER, SEPARATOR, INTEGER, SEPARATOR, 
                IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, KEYWORD, 
                IDENTIFIER, SEPARATOR, SEPARATOR, EOF_TYPE
            };
        }
};


TEST_F(code_sample2, types) {
    testTypes();
}


class code_sample3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            require "ifj21"
            function foo(x : integer, y : integer) : integer, integer
                local i : integer = x
                local j : integer = (y + 2) * 3
                i, j = j+1, i+1 -- vyhodnocuj zprava a přiřazuj později
                return i, j
            end
            function main()
                local a : integer = 1
                local b : integer = 2
                a, b = foo(a, b) -- returns 13, 2
                if a < b then
                    print(a, "<", b, "\n")
                else
                    print(a, ">=", b, "\n")
                    local a : integer = 666
                end
                print(a) --[[ prints 13 ]]
            end
            main()
            )";

            exp_types = {
                KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR,
                IDENTIFIER, SEPARATOR, KEYWORD, SEPARATOR, IDENTIFIER,
                SEPARATOR, KEYWORD, SEPARATOR, SEPARATOR, KEYWORD,
                SEPARATOR, KEYWORD, KEYWORD, IDENTIFIER, SEPARATOR,
                KEYWORD, OPERATOR, IDENTIFIER, KEYWORD, IDENTIFIER,
                SEPARATOR, KEYWORD, OPERATOR, SEPARATOR, IDENTIFIER,
                OPERATOR, INTEGER, SEPARATOR, OPERATOR, INTEGER,
                IDENTIFIER, SEPARATOR, IDENTIFIER, OPERATOR, IDENTIFIER,
                OPERATOR, INTEGER, SEPARATOR, IDENTIFIER, OPERATOR,
                INTEGER, KEYWORD, IDENTIFIER, SEPARATOR,IDENTIFIER,
                KEYWORD, KEYWORD, IDENTIFIER, SEPARATOR, SEPARATOR,
                KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD, OPERATOR,
                INTEGER, KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD, 
                OPERATOR, INTEGER, IDENTIFIER, SEPARATOR, IDENTIFIER,
                OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR,
                IDENTIFIER, SEPARATOR, KEYWORD, IDENTIFIER, OPERATOR,
                IDENTIFIER, KEYWORD, IDENTIFIER, SEPARATOR, IDENTIFIER,
                SEPARATOR, STRING, SEPARATOR, IDENTIFIER, SEPARATOR,
                STRING, SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, 
                IDENTIFIER, SEPARATOR, STRING, SEPARATOR, IDENTIFIER, 
                SEPARATOR, STRING, SEPARATOR, KEYWORD, IDENTIFIER,
                SEPARATOR, KEYWORD, OPERATOR, INTEGER, KEYWORD,
                IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, KEYWORD,
                IDENTIFIER, SEPARATOR, SEPARATOR, EOF_TYPE
                };
        }
};


TEST_F(code_sample3, types) {
    testTypes();
}


class code_sample4 : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            require "ifj21"
            function whitespaces
            ()
            local
            s
            :
            string
            =
            "\150"write(s, "\n")
            s = "a\255b"
            write(s
            ,
            "\n")x=0
            -
            1write(x)
            end whitespaces() --[[
            )";

            exp_types = {
                KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR,
                SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD,
                OPERATOR, STRING, IDENTIFIER, SEPARATOR, IDENTIFIER,
                SEPARATOR, STRING, SEPARATOR, IDENTIFIER, OPERATOR,
                STRING, IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR,
                STRING, SEPARATOR, IDENTIFIER, OPERATOR, INTEGER,
                OPERATOR, INTEGER, IDENTIFIER, SEPARATOR, IDENTIFIER,
                SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, SEPARATOR,
                ERROR_TYPE, EOF_TYPE
            };
        }
};


TEST_F(code_sample4, types) {
    testTypes();
}


class code_sample5 : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            -- Program 3: Prace s retezci a vestavenymi funkcemi
            require "ifj21"
            function main()
            local s1 : string = "Toto je nejaky text"
            local s2 : string = s1 .. ", ktery jeste trochu obohatime"
            write(s1, "\010", s2)local s1len:integer=#s1 local s1len4: integer=s1len
            s1len = s1len - 4 s1 = substr(s2, s1len, s1len4) s1len = s1len + 1
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

            exp_types = {
                KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR,
                SEPARATOR, KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD,
                OPERATOR, STRING, KEYWORD, IDENTIFIER, SEPARATOR,
                KEYWORD, OPERATOR, IDENTIFIER, OPERATOR, STRING,
                IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, STRING,
                SEPARATOR, IDENTIFIER, SEPARATOR, KEYWORD, IDENTIFIER,
                SEPARATOR, KEYWORD, OPERATOR, OPERATOR, IDENTIFIER,
                KEYWORD, IDENTIFIER, SEPARATOR, KEYWORD, OPERATOR,
                IDENTIFIER, IDENTIFIER, OPERATOR, IDENTIFIER, OPERATOR,
                INTEGER, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR,
                IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, IDENTIFIER,
                SEPARATOR, IDENTIFIER, OPERATOR, IDENTIFIER, OPERATOR,
                INTEGER, IDENTIFIER, SEPARATOR, STRING, SEPARATOR,
                IDENTIFIER, SEPARATOR, STRING, SEPARATOR, IDENTIFIER,
                SEPARATOR, STRING, SEPARATOR, IDENTIFIER, SEPARATOR,
                STRING, SEPARATOR, IDENTIFIER, SEPARATOR, STRING,
                SEPARATOR, IDENTIFIER, SEPARATOR, STRING, SEPARATOR,
                IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, SEPARATOR,
                KEYWORD, IDENTIFIER, OPERATOR, KEYWORD, KEYWORD,
                KEYWORD, IDENTIFIER, OPERATOR, STRING, KEYWORD,
                IDENTIFIER, SEPARATOR, STRING, SEPARATOR, STRING,
                SEPARATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR,
                SEPARATOR, KEYWORD, KEYWORD, KEYWORD, KEYWORD,
                IDENTIFIER, SEPARATOR, SEPARATOR, EOF_TYPE
            };

            exp_attrs = {
                "require", "\"ifj21\"", "function", "main", "(", ")", "local",
                "s1", ":", "string", "=", "\"Toto je nejaky text\"",
                "local", "s2", ":", "string", "=", "s1", "..",
                "\", ktery jeste trochu obohatime\"", "write",
                "(", "s1", "," , "\"\010\"", "s2", ")", "local",
                "s1len", ":", "integer", "=", "#", "s1", "local", "s1len4",
                ":", "integer", "=", "s1len"
            };
        }
};


TEST_F(code_sample5, types) {
    testTypes();
}

TEST_F(code_sample5, attributes) {
    testTypes();
}

class scanning_with_lookahead : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            -- Program 3: Prace s retezci a vestavenymi funkcemi
            require "ifj21"
            function main()
            local s1 : string = "Toto je nejaky text"
            local s2 : string = s1 .. ", ktery jeste trochu obohatime"
            end
            end
            main() --GSDAGDSA)";

            exp_types = {
                KEYWORD, KEYWORD, STRING, KEYWORD, KEYWORD, 
                IDENTIFIER, SEPARATOR, SEPARATOR, SEPARATOR, KEYWORD,
                KEYWORD, IDENTIFIER, SEPARATOR, SEPARATOR, KEYWORD,
                OPERATOR, OPERATOR, STRING, KEYWORD, KEYWORD,
                IDENTIFIER, SEPARATOR, SEPARATOR, KEYWORD, OPERATOR,
                OPERATOR, IDENTIFIER, OPERATOR, OPERATOR, STRING,
                KEYWORD, KEYWORD, KEYWORD, IDENTIFIER, IDENTIFIER,
                SEPARATOR, SEPARATOR, SEPARATOR, EOF_TYPE
            };
        }

        void printRow(size_t index, token_t *token, scanner_t *uut) {
            if(verbose_mode) {
                if(index % 3 == 0) {
                    printf("*");
                }

                printf("[%ld]\t%d\t%d", index, token->token_type, exp_types[index]);
                //printf("\t%lu %lu", uut.cursor_pos[ROW], uut.cursor_pos[COL]);
                printf("\t%s", get_attr(token, uut));
                printf("\n");
            }
        }

        virtual void testTypes() override {
            token_t temp;

            if(verbose_mode) {
                printf("\nI\tGot.\tExp.\tPos.\tAttr.\n");
            }

            for(size_t token_i = 0; token_i < exp_types.size(); token_i++) {
                if(token_i % 3 == 0) {
                    temp = lookahead(&uut);
                }
                else {
                    temp = get_next_token(&uut);
                }

                printRow(token_i, &temp, &uut);
                ASSERT_EQ(temp.token_type, exp_types[token_i]);

            }
        }
};


TEST_F(scanning_with_lookahead, types) {
    testTypes();
}


class comments : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            --[[Comment comment comment]]abc--def
require--abc
--[[
asdfdsaf
]]
integer
            )";

            exp_types = {
                IDENTIFIER, KEYWORD, KEYWORD, EOF_TYPE
            };

            exp_attrs = {
                "abc", "require", "integer"
            };
        }
};

TEST_F(comments, types) {
    testTypes();
}


class numbers : public test_fixture {
    protected:
        void setData() override {
            scanner_input =
            R"(
            --[[Comment comment comment]]1011010
            123.123456789 12312. 123e+1000 1213e-78798
            1232.987e1233 123112e12321 65456.46456e-5465
            544645.5465e+87987 89787e
            )";

            exp_types = {
                INTEGER, NUMBER, ERROR_TYPE, NUMBER, NUMBER,
                NUMBER, NUMBER, NUMBER, NUMBER, ERROR_TYPE,
                EOF_TYPE
            };
        }
};

TEST_F(numbers, types) {
    testTypes();
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


/***                            End of scanner_test.cpp                    ***/

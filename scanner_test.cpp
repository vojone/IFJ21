/******************************************************************************
 *                                  IFJ21
 *                             scanner_test.cpp
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *           Purpose: Unit tests for scanner
 * 
 *                  Last change: 
 *****************************************************************************/ 

/**
 * @file scanner_test.c
 * @brief Unit tests for scanner
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#define TMP_FILE_NAME "tmp.inp" 

extern "C" {
    #include "scanner.h"
}

#include "gtest/gtest.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

FILE * create_input_file(std::string *name, std::string *content) {
    FILE * test_file = fopen(name->c_str(), "r");
    if(test_file) {
        fprintf(stderr, "Scanner test: Err: File %s exists!\n", name->c_str());

        return NULL;
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

class test_fixture : public ::testing::Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input;
        std::vector<token_type_t> exp_types;

        scanner_t dut;
        bool init_success;

        virtual void setData() {
            scanner_input = "";
            exp_types = {EOF_TYPE};
        }

        virtual void testTypes() {
            token_t temp;

            printf("\nI\tGot.\tExp.\n");
            for(size_t i = 0; i < exp_types.size(); i++) {
                temp = get_next_token(&dut);

                printf("[%ld]\t%d\t%d\n", i, temp.token_type, exp_types[i]);
                ASSERT_EQ(exp_types[i], temp.token_type);
            }
        }

        virtual void SetUp() {
            setData(); 
            init_success = prepare_tests(&inp_filename, &scanner_input, &dut);
        }

        virtual void TearDown() {
            if(init_success) {
                remove(inp_filename.c_str());
            }
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
            // /
            #"length")";
            exp_types = {IDENTIFIER, IDENTIFIER, INTEGER, 
                         NUMBER, NUMBER, SEPARATOR, 
                         SEPARATOR, STRING, OPERATOR, OPERATOR, 
                         OPERATOR, OPERATOR, OPERATOR, OPERATOR, 
                         OPERATOR, OPERATOR, OPERATOR, OPERATOR, 
                         OPERATOR, STRING, EOF_TYPE};
        };
};

TEST_F(random_tokens, types) {
    testTypes();
}


class whitespaces : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(--[[]])";

            exp_types = {EOF_TYPE};
        }
};


TEST_F(whitespaces, types) {
    testTypes();
}



class errors : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(  ; ; ; ; === ")";
            exp_types = {ERROR_TYPE, ERROR_TYPE, ERROR_TYPE, ERROR_TYPE,
                         OPERATOR, OPERATOR, ERROR_TYPE, EOF_TYPE};
        }
};


TEST_F(errors, types) {
    testTypes();
}


class code_sample : public test_fixture {
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
            KEYWORD, STRING, KEYWORD, IDENTIFIER, SEPARATOR, SEPARATOR, 
            KEYWORD,  IDENTIFIER, SEPARATOR, KEYWORD, KEYWORD, 
            IDENTIFIER, SEPARATOR, KEYWORD, OPERATOR, INTEGER, 
            IDENTIFIER, SEPARATOR, STRING, SEPARATOR, IDENTIFIER, 
            OPERATOR, IDENTIFIER, SEPARATOR, SEPARATOR, KEYWORD, 
            IDENTIFIER, OPERATOR, KEYWORD, KEYWORD, IDENTIFIER, 
            SEPARATOR, STRING, SEPARATOR, KEYWORD}; //write("a je nil\n") return
        }
};


TEST_F(code_sample, types) {
    testTypes();
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}


/***                            End of scanner_test.cpp                    ***/

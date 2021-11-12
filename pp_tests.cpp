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
 * @file scanner_test.c
 * @brief Unit tests for precedence parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#define TMP_FILE_NAME "tmp.inp" /**< Temporary file for testing inputs (stream from this file is redirected to stdin)*/

bool verbose_mode = false; /**< Tests prints only test cases with differences between expected result and got */
bool force_mode = false; /**< Tests automatically prevents overwriting files, if is set to false */

extern "C" {
    #include "precedence_parser.h"
}

#include "gtest/gtest.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

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

class test_fixture : public :: testing :: Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input;

        scanner_t uut;
        bool init_success;

        virtual void setData() {
            scanner_input = R"(
                (a) -12 * (-1 + b) 
            )";
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

TEST_F(test_fixture, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_SUCCESS);
}


class lexical_error : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a + ;b * 5 
            )";
        }
};

TEST_F(lexical_error, only_parse) {
    ASSERT_EQ(parse_expression(&uut), LEXICAL_ERROR);
}

class basic : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(8+8*42
            )";
        }
};

TEST_F(basic, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_SUCCESS);
}

class str_parse : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(#("A".."B".."D".."C") + #("D".."E".."F".."G")
            )";
        }
};

TEST_F(str_parse, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_SUCCESS);
}

class nested_parenthesis : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(((8+8))*7+((1+1)*a)
            )";
        }
};

TEST_F(nested_parenthesis, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_SUCCESS);
}

class relational_operators : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(8 + 8 >= 42*42
            )";
        }
};

TEST_F(relational_operators, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_SUCCESS);
}


class parenthesis_err : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"((8+8)(42+42)
            )";
        }
};

TEST_F(parenthesis_err, only_parse) {
    ASSERT_EQ(parse_expression(&uut), EXPRESSION_FAILURE);
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

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

class random_tokens : public ::testing::Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input = "abc number 45 1231.456 12311e2 ( ) \"s\"";
        std::vector<token_type_t> exp_types = {IDENTIFIER, KEYWORD, INTEGER, 
                                               NUMBER, NUMBER, SEPARATOR, 
                                               SEPARATOR, STRING, EOF_TYPE};

        scanner_t dut;
        bool init_success;

        virtual void SetUp() {
            init_success = prepare_tests(&inp_filename, &scanner_input, &dut);
        }

        virtual void TearDown() {
            if(init_success) {
                remove(inp_filename.c_str());
            }
        }
};



TEST_F(random_tokens, basics) {
    token_t temp;

    for(size_t i = 0; i < exp_types.size(); i++) {
        temp.token_type = exp_types[i];

        ASSERT_EQ(temp.token_type, get_next_token(&dut).token_type);
    }
}


class whitespaces : public ::testing::Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input = "\t\r\n\n\n             42    \n \n 56\n";
        std::vector<token_type_t> exp_types = {INTEGER, INTEGER, EOF_TYPE};

        scanner_t dut;
        bool init_success;

        virtual void SetUp() {
            init_success = prepare_tests(&inp_filename, &scanner_input, &dut);
        }

        virtual void TearDown() {
            if(init_success) {
                remove(inp_filename.c_str());
            }
        }
};


TEST_F(whitespaces, basics) {
    token_t temp;

    for(size_t i = 0; i < exp_types.size(); i++) {
        temp.token_type = exp_types[i];

        ASSERT_EQ(temp.token_type, get_next_token(&dut).token_type);
    }
}



class errors : public ::testing::Test {
    protected:
        std::string inp_filename = TMP_FILE_NAME;
        std::string scanner_input = "; ; ; \n 656abc \n 1110a11 \"";
        std::vector<token_type_t> exp_types = {ERROR_TYPE, ERROR_TYPE, 
                                               ERROR_TYPE, ERROR_TYPE,
                                               ERROR_TYPE, ERROR_TYPE,
                                               EOF_TYPE};

        scanner_t dut;
        bool init_success;

        virtual void SetUp() {
            init_success = prepare_tests(&inp_filename, &scanner_input, &dut);
        }

        virtual void TearDown() {
            if(init_success) {
                remove(inp_filename.c_str());
            }
        }
};


TEST_F(errors, basics) {
    token_t temp;

    for(size_t i = 0; i < exp_types.size(); i++) {
        temp.token_type = exp_types[i];

        ASSERT_EQ(temp.token_type, get_next_token(&dut).token_type);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}


/***                            End of scanner_test.cpp                    ***/

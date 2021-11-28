#define TMP_FILE_NAME "tmp.inp" /**< Temporary file for testing inputs (stream from this file is redirected to stdin)*/

bool verbose_mode = false; /**< Tests prints only test cases with differences between expected result and got */
bool force_mode = false; /**< Tests automatically prevents overwriting files, if is set to false */

extern "C" {
    #include "generator.h"
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
        std::vector<std::string> input;
        string_t ret_type;

        prog_t program;

        virtual void setData() {
        }


        virtual void SetUp() {
            setData(); 
            init_new_prog(&program);
        }

        virtual void TearDown() {
            print_program(&program);
            program_dtor(&program);
        }
};


/** USAGE OF INTERNAL REPRESENTATION **/
//make unit_tests && ./gen_tests to see result
TEST_F(test_fixture, only_parse) {
    //Dont forget to call init_program() before
    app_instr(&program, "Ahoj %a", 4.1); //<- stop
    instr_t *stop = get_last(&program);
    app_instr(&program, "SVETE");
    app_instr(&program, "?");
    app_instr(&program, "!");

    ins_before(&program, stop, "PUSHS");

    revert(&program, stop, get_last(&program));

    stop = get_prev(stop);

    ins_after(&program, stop, "POPS");
    //Dont forget to call program_dtor()
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

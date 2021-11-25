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
        sym_dtype_t ret_type;

        scanner_t uut;
        symbol_tables_t syms;
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

            init_tab(&syms.symtab);
            init_tab(&syms.global);
            symtabs_stack_init(&syms.symtab_st);
        }

        virtual void TearDown() {
            remove(inp_filename.c_str());
            scanner_dtor(&uut);

            symtabs_stack_dtor(&syms.symtab_st);
            destroy_tab(&syms.symtab);
            destroy_tab(&syms.global);
        }
};

TEST_F(test_fixture, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), UNDECLARED_IDENTIFIER);
}


class lexical_error : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4 + ;1 * 5 
            )";
        }
};

TEST_F(lexical_error, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), LEXICAL_ERROR);
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
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, INT);
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
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, INT);
}

class nested_parenthesis : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(((8+8))*7+((1+1)*10)
            )";
        }
};

TEST_F(nested_parenthesis, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}

class relational_operators1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(8 + 8 >= 42*42
            )";
        }
};

TEST_F(relational_operators1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, BOOL);
}


class relational_operators2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"("abc" == "def"
            )";
        }
};

TEST_F(relational_operators2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, BOOL);
}


class relational_operators3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"("abc" > 7
            )";
        }
};

TEST_F(relational_operators3, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), SEM_ERROR_IN_EXPR);
}

class relational_operators4 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(42 > "abc"
            )";
        }
};

TEST_F(relational_operators4, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), SEM_ERROR_IN_EXPR);
}


class relational_operators5 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4.2 < 7
            )";
        }
};

TEST_F(relational_operators5, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, BOOL);
}


class relational_operators6 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4.2 ~= "str"
            )";
        }
};

TEST_F(relational_operators6, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), SEM_ERROR_IN_EXPR);
}

class relational_operators7 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(#("def".."abc") ~= "str"
            )";
        }
};

TEST_F(relational_operators7, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), SEM_ERROR_IN_EXPR);
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
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class op_err1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(42++42
            )";
        }
};

TEST_F(op_err1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}

class op_err2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(42**42
            )";
        }
};

TEST_F(op_err2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}


class multiple_call1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(42 42
            )";
        }
};

TEST_F(multiple_call1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, INT);
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, INT);
}

class multiple_call2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a+b b//b
            )";
        }
};

TEST_F(multiple_call2, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    insert_sym(&syms.symtab, "b", {{0, 0, (char *)"b"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, NUM);
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, INT);
}

class op_no_err1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(7 + 4 1 = 4 + 1 
            )";
        }
};

TEST_F(op_no_err1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS); /**< PP parser should return control to the topdown parser*/
}

class op_no_err2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(1 + 2 = c
            )";
        }
};

TEST_F(op_no_err2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS); /**< PP parser should return control to the topdown parser*/
}

class unary_minus : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(42//42--42
            )";
        }
};

TEST_F(unary_minus, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


/**************************TESTS WITH SYMTABLE********************************/

class undeclared1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a 
            )";
        }
};

TEST_F(undeclared1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), UNDECLARED_IDENTIFIER);
}

class undeclared2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a + b a = a + 1 
            )";
        }
};

TEST_F(undeclared2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), UNDECLARED_IDENTIFIER);
}

class undeclared3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a + b = c
            )";
        }
};

TEST_F(undeclared3, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), UNDECLARED_IDENTIFIER); /**< PP parser should return control to the topdown parser*/
}

class nested_parenthesis_undeclared : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(((8+8))*7+((1+1)*a)
            )";
        }
};

TEST_F(nested_parenthesis_undeclared, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), UNDECLARED_IDENTIFIER);
}

class declared1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a 
            )";
        }
};

TEST_F(declared1, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class declared2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a + b
            )";
        }
};

TEST_F(declared2, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    insert_sym(&syms.symtab, "b", {{0, 0, (char *)"b"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
    ASSERT_EQ(ret_type, NUM);
}


class declared3_sem_err : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a + b
            )";
        }
};

TEST_F(declared3_sem_err, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    insert_sym(&syms.symtab, "b", {{0, 0, (char *)"b"}, VAR, {0, 0, NULL}, {0, 0, NULL}, STR, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), SEM_ERROR_IN_EXPR);
}

class nil1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a ~= nil
            )";
        }
};

TEST_F(nil1, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


/*****************************************************************************/

class nil2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"("abc" == nil)
            )";
        }
};

TEST_F(nil2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class nil3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(nil + 5
            )";
        }
};

TEST_F(nil3, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), NIL_ERROR);
}


class zero1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"((0//5)/1
            )";
        }
};

TEST_F(zero1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class zero2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4/0
            )";
        }
};

TEST_F(zero2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), DIV_BY_ZERO);
}


class zero3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(a//0
            )";
        }
};

TEST_F(zero3, only_parse) {
    insert_sym(&syms.symtab, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), DIV_BY_ZERO);
}


class zero4 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4/((0+7)*0)
            )";
        }
};

TEST_F(zero4, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), DIV_BY_ZERO);
}


class zero5 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4/(0+(-0))
            )";
        }
};

TEST_F(zero5, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), DIV_BY_ZERO);
}



class missing_par : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(4/((0+(-0))
            )";
        }
};

TEST_F(missing_par, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}




class missing_par1 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"((()
            )";
        }
};

TEST_F(missing_par1, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}




class missing_par2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"((4+(8)
            )";
        }
};

TEST_F(missing_par2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}




class missing_par3 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"((((((((((((((((((((((((((((((4/())))))))))))))))))))))))))))))
            )";
        }
};

TEST_F(missing_par3, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_FAILURE);
}


class missing_par4 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(((48+(8))))
            )";
        }
};

TEST_F(missing_par4, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class minuses : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(-(-(-(-(-8))))
            )";
        }
};

TEST_F(minuses, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
}


class minuses2 : public test_fixture {
    protected:
        void setData() override {
            scanner_input = 
            R"(8 + - - - - - - 7
            )";
        }
};

TEST_F(minuses2, only_parse) {
    ASSERT_EQ(parse_expression(&uut, &syms, &ret_type), EXPRESSION_SUCCESS);
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

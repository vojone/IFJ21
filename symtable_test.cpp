/******************************************************************************
 *                                  IFJ21
 *                             symtable_test.cpp
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *                 Purpose: Unit tests for symtable
 * 
 *                      Last change: 30. 10. 2021
 *****************************************************************************/ 

/**
 * @file symtable_test.c
 * @brief Unit tests for symtable
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include "gtest/gtest.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

bool verbose_mode = false;

extern "C" {
    #include "symtable.h"
}

void print_node(tree_node_t *node) {
    fprintf(stderr, "Key: %s\n", node->key);
    fprintf(stderr, "Data: %s",node->data.name);
    fprintf(stderr, " %d", node->data.dtype);
    fprintf(stderr, " %d", node->data.type);
    fprintf(stderr, " %d\n", node->data.status);

    if(node->lPtr) {
        fprintf(stderr, "lPtr: %s\n", node->lPtr->key);
    }
    else {
        fprintf(stderr, "lPtr key: NULL\n");
    }

    if(node->rPtr) {
        fprintf(stderr, "rPtr: %s\n", node->rPtr->key);
    }
    else {
        fprintf(stderr, "rPtr key: NULL\n");
    }

    fprintf(stderr, "---------------------------------------------------");
}

void print_tab(tree_node_t *node) {
    if(node != NULL) {
        print_node(node);
        print_tab(node->lPtr);
        print_tab(node->rPtr);
    }
}

class empty_symtable : public ::testing::Test {
    protected:
        symtab_t uut;

    virtual void SetUp() {
        init_tab(&uut);
    }

    virtual void TearDown() {
        if(verbose_mode) {
            print_tab(uut);
        }

        destroy_tab(&uut);
    }
};

TEST_F(empty_symtable, insert) {
    ASSERT_EQ(uut, (void *)NULL);

    insert_sym(&uut, "new");
    ASSERT_EQ(uut->key, "new");
    ASSERT_EQ(uut->lPtr, (void *)NULL);
    ASSERT_EQ(uut->rPtr, (void *)NULL);
}

TEST_F(empty_symtable, search) {
    ASSERT_EQ((void *)NULL, search(&uut, "inserted"));
    insert_sym(&uut, "inserted");
    ASSERT_EQ(uut, search(&uut, "inserted"));

    insert_sym(&uut, "a");
    ASSERT_NE((void *)NULL, search(&uut, "a"));
}

TEST_F(empty_symtable, delete_s) {
    delete_sym(&uut, "nonexisting");

    insert_sym(&uut, "inserted");
    ASSERT_EQ(uut->key, "inserted");
    delete_sym(&uut, "inserted");
    ASSERT_EQ(uut, (void *)NULL);
}

TEST_F(empty_symtable, set) {
    insert_sym(&uut, "new");
    sym_data_t new_symbol_data = {(char *)"new", VAR, NUMBER, DECLARED};
    set_sym(&uut, "new", new_symbol_data);
    ASSERT_EQ(uut->data.name, "new");
    ASSERT_EQ(uut->data.dtype, NUMBER);
    ASSERT_EQ(uut->data.type, VAR);
    ASSERT_EQ(uut->data.status, DECLARED);
}


class normal_tests : public ::testing::Test {
    protected:
        symtab_t uut;
        std::vector<sym_data_t> symbols = {
            {(char *)"Car", FUNC, INTEGER, DECLARED},
            {(char *)"Cat", VAR, INTEGER, DECLARED},
            {(char *)"Can", VAR, INTEGER, DEFINED},
            {(char *)"Dog", VAR, NUMBER, DECLARED},
            {(char *)"Fish", VAR, STRING, USED},
        };

    virtual void SetUp() {
        init_tab(&uut);

        for(size_t i = 0; i < symbols.size(); i++) {
            insert_sym(&uut, symbols[i].name);
            set_sym(&uut, symbols[i].name, symbols[i]);
        }
    }

    virtual void TearDown() {
        if(verbose_mode) {
            print_tab(uut);
        }

        if(uut != NULL) {
            destroy_tab(&uut);
        }
    }
};


TEST_F(normal_tests, search) {
    for(size_t i = 0; i < symbols.size(); i++) {
        tree_node_t* current = search(&uut, symbols[i]. name);
        ASSERT_EQ(current->key, symbols[i].name);
    }
}


TEST_F(normal_tests, delete_test) {
    delete_sym(&uut, symbols[0].name);
    ASSERT_EQ(search(&uut, symbols[0].name), (void *)NULL);

    delete_sym(&uut, symbols[symbols.size() - 1].name);
    ASSERT_EQ(search(&uut, symbols[symbols.size() - 1].name), (void *)NULL);
}


TEST_F(normal_tests, dispose_whole_table) {
    destroy_tab(&uut);
    ASSERT_EQ(uut, (void *)NULL);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

    if(argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose_mode = true;
    }

    return RUN_ALL_TESTS();
}



/***                            End of symtable_test.cpp                   ***/

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
    fprintf(stderr, "Data: %s", to_str(&node->data.name));
    fprintf(stderr, " %d", node->data.dtype);
    fprintf(stderr, " %d", node->data.type);
    fprintf(stderr, " %d\n", node->data.status);

    if(node->l_ptr) {
        fprintf(stderr, "lPtr: %s\n", node->l_ptr->key);
    }
    else {
        fprintf(stderr, "lPtr key: NULL\n");
    }

    if(node->r_ptr) {
        fprintf(stderr, "rPtr: %s\n", node->r_ptr->key);
    }
    else {
        fprintf(stderr, "rPtr key: NULL\n");
    }

    fprintf(stderr, "---------------------------------------------------\n");
}

void print_tab(tree_node_t *node) {
    if(node != NULL) {
        print_node(node);
        print_tab(node->l_ptr);
        print_tab(node->r_ptr);
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
            print_tab(uut.t);
        }

        destroy_tab(&uut);
    }
};

TEST_F(empty_symtable, insert) {
    ASSERT_EQ(uut.t, (void *)NULL);
    insert_sym(&uut, "new", {{0, 0, (char *)"new"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    ASSERT_EQ(strcmp(uut.t->key, "new"), 0);
    ASSERT_EQ(uut.t->l_ptr, (void *)NULL);
    ASSERT_EQ(uut.t->r_ptr, (void *)NULL);
}

TEST_F(empty_symtable, search) {
    ASSERT_EQ((void *)NULL, search(&uut, "inserted"));
    insert_sym(&uut, "inserted", {{0, 0, (char *)"inserted"}, VAR,  {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    ASSERT_EQ(uut.t, search(&uut, "inserted"));

    insert_sym(&uut, "a", {{0, 0, (char *)"a"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    ASSERT_NE((void *)NULL, search(&uut, "a"));
}

TEST_F(empty_symtable, delete_s) {
    delete_sym(&uut, "nonexisting");

    insert_sym(&uut, "inserted", {{0, 0, (char *)"inserted"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED});
    ASSERT_EQ(strcmp(uut.t->key, "inserted"), 0);
    delete_sym(&uut, "inserted");
    ASSERT_EQ(uut.t, (void *)NULL);
}

TEST_F(empty_symtable, set) {
    sym_data_t new_symbol_data = {{0, 0, (char *)"new"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED};
    insert_sym(&uut, "new", new_symbol_data);
    ASSERT_EQ(strcmp(uut.t->key, "new"), 0);
    ASSERT_EQ(uut.t->data.dtype, NUM);
    ASSERT_EQ(uut.t->data.type, VAR);
    ASSERT_EQ(uut.t->data.status, DECLARED);
}


class normal_tests1 : public ::testing::Test {
    protected:
        symtab_t uut;
        std::vector<sym_data_t> symbols = {
            {{0, 0, (char *)"Car"}, FUNC, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED},
            {{0, 0, (char *)"Cat"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED},
            {{0, 0, (char *)"Can"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DEFINED},
            {{0, 0, (char *)"Dog"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED},
            {{0, 0, (char *)"Fish"}, VAR, {0, 0, NULL}, {0, 0, NULL}, STR, DECLARED},
        };

    virtual void SetUp() {
        init_tab(&uut);
        for(size_t i = 0; i < symbols.size(); i++) {
            insert_sym(&uut, to_str(&(symbols[i].name)), symbols[i]);
        }
        
    }

    virtual void TearDown() {
        if(verbose_mode) {
            print_tab(uut.t);
        }

        if(uut.t != NULL) {
            destroy_tab(&uut);
        }
    }
};


TEST_F(normal_tests1, search) {
    for(size_t i = 0; i < symbols.size(); i++) {
        tree_node_t* current = search(&uut, to_str(&symbols[i].name));
        ASSERT_EQ(str_cmp(current->key, to_str(&symbols[i].name)), 0);
    }
}


TEST_F(normal_tests1, delete_test) {
    delete_sym(&uut, to_str(&symbols[0].name));
    ASSERT_EQ(search(&uut, to_str(&symbols[0].name)), (void *)NULL);
    ASSERT_NE(search(&uut, to_str(&symbols[1].name)), (void *)NULL);
    ASSERT_NE(search(&uut, to_str(&symbols[symbols.size() - 1].name)), (void *)NULL);
    delete_sym(&uut, to_str(&symbols[symbols.size() - 1].name));
    ASSERT_EQ(search(&uut, to_str(&symbols[symbols.size() - 1].name)), (void *)NULL);
}


TEST_F(normal_tests1, dispose_whole_table) {
    destroy_tab(&uut);
    ASSERT_EQ(uut.t, (void *)NULL);
}


class normal_tests2 : public ::testing::Test {
    protected:
        symtab_t uut;
        std::vector<sym_data_t> symbols = {
            {{0, 0, (char *)"F"}, FUNC, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED},
            {{0, 0, (char *)"B"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED},
            {{0, 0, (char *)"C"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DEFINED},
            {{0, 0, (char *)"A"}, VAR, {0, 0, NULL}, {0, 0, NULL}, NUM, DECLARED},
            {{0, 0, (char *)"E"}, VAR, {0, 0, NULL}, {0, 0, NULL}, STR, DEFINED},
            {{0, 0, (char *)"D"}, VAR, {0, 0, NULL}, {0, 0, NULL}, STR, DEFINED},
        };

    virtual void SetUp() {
        init_tab(&uut);
        for(size_t i = 0; i < symbols.size(); i++) {
            insert_sym(&uut, to_str(&(symbols[i].name)), symbols[i]);
        }
        
    }

    virtual void TearDown() {
        if(verbose_mode) {
            print_tab(uut.t);
        }

        if(uut.t != NULL) {
            destroy_tab(&uut);
        }
    }
};


TEST_F(normal_tests2, delete_test) {
    delete_sym(&uut, "B"); //Deletion of node with both subtrees
    ASSERT_NE(search(&uut, "A"), (void *)NULL);
    ASSERT_NE(search(&uut, "E"), (void *)NULL);
    ASSERT_NE(search(&uut, "C"), (void *)NULL);
    ASSERT_NE(search(&uut, "F"), (void *)NULL);
    ASSERT_NE(search(&uut, "D"), (void *)NULL);

    delete_sym(&uut, "E");
    ASSERT_NE(search(&uut, "A"), (void *)NULL);
    ASSERT_NE(search(&uut, "C"), (void *)NULL);
    ASSERT_NE(search(&uut, "F"), (void *)NULL);
    ASSERT_NE(search(&uut, "D"), (void *)NULL);

    delete_sym(&uut, "F");
    ASSERT_NE(search(&uut, "A"), (void *)NULL);
    ASSERT_NE(search(&uut, "C"), (void *)NULL);
    ASSERT_NE(search(&uut, "D"), (void *)NULL);

    destroy_tab(&uut);
    ASSERT_EQ(uut.t, (void *)NULL);
}

TEST_F(normal_tests2, insert_test) {
    insert_sym(&uut, "Blue", {{0, 0, (char*)"Blue"}, VAR, {0, 0, NULL}, {0, 0, NULL}, INT, DECLARED});
    ASSERT_NE(search(&uut, "Blue"), (void *)NULL);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

    if(argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose_mode = true;
    }

    return RUN_ALL_TESTS();
}



/***                            End of symtable_test.cpp                   ***/

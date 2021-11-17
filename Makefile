EXECUTABLE = IFJ21

SCANNER = scanner
SYMTAB = symtable

CC = gcc
CFLAGS = -Werror -Wall -pedantic -std=c99



PARSER_EXE = IFJ21Parser
PARSER = parser_topdown
PP_PARSER = precedence_parser

PP_TEST_NAME = pp_tests
PP_TEST_BIN = $(PP_TEST_NAME)

#--------------------------------------TESTS-----------------------------------

#testing framework path
TESTLIB_NAME = libgtest
TEST_DIR = googletest-master/googletest/

CXX = g++
CXXFLAGS := -Werror -Wall -pedantic -std=c++11

PARSER_TEST_NAME = parser_topdown_tests
PARSER_TEST_BIN = $(PARSER_TEST_NAME)

#------------------------------------------------------------------------------

OBJS = parser_wrapper.o $(PARSER).o $(PP_PARSER).o $(SCANNER).o dstring.o tables.o dstack.o
EXES = $(EXECUTABLE) $(PARSER_TEST_BIN) $(SYMTAB_TEST_BIN) $(PARSER_EXE)

.PHONY: all clean unit_tests

parser: $(OBJS) 
	$(CC) $(CFLAGS) -o $(PARSER_EXE) $^

clean:
	rm -f *.o $(EXES)
	



unit_tests: $(SCAN_TEST_BIN) $(PARSER_TEST_BIN) $(SYMTAB_TEST_BIN)



#--------------------- PARSER - TOPDOWN TESTS-----------------------------

#linking binary with test
$(PARSER_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(PARSER_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(PARSER_TEST_BIN) : $(PARSER).o $(PARSER_TEST_BIN).o $(PP_PARSER).o $(SCANNER).o dstring.o tables.o dstack.o

#compilation of obj file with test
$(PARSER_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include
$(PARSER_TEST_BIN).o : $(PARSER_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

$(TEST_DIR)lib/%.a :
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------

$(TEST_DIR)lib/%.a :
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------

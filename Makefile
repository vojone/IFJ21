EXECUTABLE = IFJ21

SCANNER = scanner
SYMTAB = symtable

CC = gcc
CFLAGS = -Werror -Wall -pedantic -std=c99

PARSER_EXE = IFJ21Parser
PARSER = parser-topdown
PP_PARSER = precedence_parser

#--------------------------------------TESTS-----------------------------------

#testing framework path
TESTLIB_NAME = libgtest
TEST_DIR = googletest-master/googletest/

CXX = g++
CXXFLAGS := -Werror -Wall -pedantic -std=c++11

SCAN_TEST_NAME = scanner_test
SCAN_TEST_BIN = $(SCAN_TEST_NAME)

SYMTAB_TEST_NAME = symtable_test
SYMTAB_TEST_BIN = $(SYMTAB_TEST_NAME)

PP_TEST_NAME = pp_tests
PP_TEST_BIN = $(PP_TEST_NAME)

#------------------------------------------------------------------------------

OBJS = $(PARSER).o $(SCANNER).o dstring.o tables.o dstack.o
EXES = $(EXECUTABLE) $(SCAN_TEST_BIN) $(PP_TEST_BIN) $(SYMTAB_TEST_BIN) $(PARSER_EXE)

.PHONY: all clean unit_tests

all :  $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

clean:
	rm -f *.o $(EXES)
	
parser: $(OBJS) 
	$(CC) $(CFLAGS) -o $(PARSER_EXE) $^


unit_tests: $(SCAN_TEST_BIN) $(PP_TEST_BIN) $(SYMTAB_TEST_BIN)



#------------------------------SCANNER TESTS-----------------------------------

#linking binary with test
$(SCAN_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(SCAN_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(SCAN_TEST_BIN) : $(SCANNER).o $(SCAN_TEST_BIN).o dstring.o tables.o dstack.o

#compilation of obj file with test
$(SCAN_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(SCAN_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a
#--------------------------------SYMTAB TESTS----------------------------------

#linking binary with test
$(SYMTAB_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(SYMTAB_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(SYMTAB_TEST_BIN) : $(SYMTAB).o $(SYMTAB_TEST_BIN).o dstring.o tables.o dstack.o

#compilation of obj file with test
$(SYMTAB_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(SYMTAB_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a


#---------------------PRECEDENCE PARSER (PP) TESTS-----------------------------

#linking binary with test
$(PP_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(PP_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib 
$(PP_TEST_BIN) : $(PP_PARSER).o $(PP_TEST_BIN).o $(SCANNER).o dstring.o tables.o dstack.o

#compilation of obj file with test
$(PP_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(PP_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

$(TEST_DIR)lib/%.a :
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------

$(TEST_DIR)lib/%.a :
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------

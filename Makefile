EXECUTABLE = IFJ21


ZIPNAME = xdvora3o

CC = gcc
CFLAGS = -Werror -Wall -pedantic -std=c99


SCANNER = scanner
SYMTAB = symtable
PARSER = parser_topdown

PP_PARSER = precedence_parser

PARSER_EXE = IFJ21Parser

INZIP = *.c *.h Makefile rozdeleni rozsireni dokumentace.pdf README.md


#--------------------------------------TESTS-----------------------------------

#testing framework path
TESTLIB_NAME = libgtest
TEST_DIR = googletest/googletest/

INT_TEST = ifjtest
INT_TEST_DIR = ifjtest/

CXX = g++
CXXFLAGS := -Werror -Wall -pedantic -std=c++11

PARSER_TEST_NAME = parser_topdown_tests
PARSER_TEST_BIN = $(PARSER_TEST_NAME)

PP_TEST_NAME = pp_tests
PP_TEST_BIN = $(PP_TEST_NAME)

SYMTAB_TEST_NAME = symtable_tests
SYMTAB_TEST_BIN = $(SYMTAB_TEST_NAME)

SCAN_TEST_NAME = scanner_tests
SCAN_TEST_BIN = $(SCAN_TEST_NAME)

GEN_TEST_NAME = gen_tests
GEN_TEST_BIN = $(GEN_TEST_NAME)

#------------------------------------------------------------------------------

OBJS = $(PARSER).o $(PP_PARSER).o $(SCANNER).o $(SYMTAB).o \
	   main.o dstring.o tables.o generator.o

EXES = $(EXECUTABLE) $(PARSER_TEST_BIN) $(SCAN_TEST_BIN) $(PP_TEST_BIN) \
	   $(SYMTAB_TEST_BIN) $(GEN_TEST_NAME) $(PARSER_EXE)

.PHONY: all parser generator clean unit_tests test

all : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

parser: $(OBJS)
	$(CC) $(CFLAGS) -o $(PARSER_EXE) $^

generator: generator_wrapper.o generator.o dstring.o  $(SYMTAB).o $(SCANNER).o $(PP_PARSER).o tables.o
	$(CC) $(CFLAGS) -o generator $^

clean:
	rm -f *.o $(EXES) $(ZIPNAME).zip
	rm -f ifjtest/tmp/*

zip: clean
	zip $(ZIPNAME).zip $(INZIP)
	

unit_tests:  $(PARSER_TEST_BIN) $(SCAN_TEST_BIN) $(PP_TEST_BIN) \
	   		 $(SYMTAB_TEST_BIN) $(GEN_TEST_BIN)
#			 ./$(PARSER_TEST_BIN)
#			 ./$(SCAN_TEST_BIN)
#		 	 ./$(PP_TEST_BIN)
#			./$(SYMTAB_TEST_BIN)


$(TEST_DIR) :
	git clone https://github.com/google/googletest.git


#--------------------- PARSER - TOPDOWN TESTS-----------------------------

#linking binary with test
$(PARSER_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(PARSER_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(PARSER_TEST_BIN) : $(PARSER).o $(PARSER_TEST_BIN).o $(SCANNER).o $(SYMTAB).o \
					 $(PP_PARSER).o dstring.o tables.o  generator.o

#compilation of obj file with test
$(PARSER_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include
$(PARSER_TEST_BIN).o : $(PARSER_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a



#------------------------------SCANNER TESTS-----------------------------------

#linking binary with test
$(SCAN_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(SCAN_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(SCAN_TEST_BIN) : $(SCANNER).o $(SCAN_TEST_BIN).o dstring.o tables.o 

#compilation of obj file with test
$(SCAN_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(SCAN_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

#--------------------------------SYMTAB TESTS----------------------------------

#linking binary with test
$(SYMTAB_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(SYMTAB_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(SYMTAB_TEST_BIN) : $(SYMTAB).o $(SYMTAB_TEST_BIN).o \
					 dstring.o tables.o 

#compilation of obj file with test
$(SYMTAB_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(SYMTAB_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a


#---------------------PRECEDENCE PARSER (PP) TESTS-----------------------------

#linking binary with test
$(PP_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(PP_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(PP_TEST_BIN) : $(SYMTAB).o $(PP_PARSER).o $(PP_TEST_BIN).o $(SCANNER).o \
				 dstring.o tables.o  generator.o

#compilation of obj file with test
$(PP_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include
$(PP_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

#------------------------------------------------------------------------------


#-----------------------------GENERATOR (PP) TESTS-----------------------------

#linking binary with test
$(GEN_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(GEN_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(GEN_TEST_BIN) : $(SYMTAB).o $(PP_PARSER).o $(GEN_TEST_BIN).o $(SCANNER).o \
				 dstring.o tables.o  generator.o

#compilation of obj file with test
$(GEN_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include
$(GEN_TEST_BIN).o : $(GEN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

#------------------------------------------------------------------------------

$(TEST_DIR)lib/%.a : $(TEST_DIR)
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------


EXECUTABLE = IFJ21

SCANNER = scanner

CC = gcc
CFLAGS = -Werror -Wall -pedantic -std=c99

#--------------------------------------TESTS-----------------------------------

#testing framework path
TESTLIB_NAME = libgtest
TEST_DIR = googletest-master/googletest/

CXX = g++
CXXFLAGS := -Werror -Wall -pedantic -std=c++11

SCAN_TEST_NAME = scanner_test
SCAN_TEST_BIN = $(SCAN_TEST_NAME)

#------------------------------------------------------------------------------

OBJS = $(SCANNER).o
EXES = $(EXECUTABLE) $(SCAN_TEST_BIN)

.PHONY: all clean unit_tests

all :  $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

clean:
	rm -f *.o $(EXES)
	

unit_tests: $(SCAN_TEST_BIN)
	./$(SCAN_TEST_BIN)



#------------------------------SCANNER TESTS-----------------------------------

#linking binary with test
$(SCAN_TEST_BIN) : LDLIBS := -L$(TEST_DIR)lib -lgtest -lpthread -lstdc++ -lm
$(SCAN_TEST_BIN) : LDFLAGS := -L$(TEST_DIR)lib
$(SCAN_TEST_BIN) : $(SCANNER).o $(SCAN_TEST_BIN).o

#compilation of obj file with test
$(SCAN_TEST_BIN).o : CXXFLAGS := $(CXXFLAGS) -I$(TEST_DIR)include 
$(SCAN_TEST_BIN).o : $(SCAN_TEST_NAME).cpp $(TEST_DIR)lib/$(TESTLIB_NAME).a

$(TEST_DIR)lib/%.a :
	cd $(TEST_DIR) && cmake .. && make -s

#------------------------------------------------------------------------------

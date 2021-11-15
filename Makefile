EXECUTABLE = IFJ21Parser

SCANNER = scanner
PARSER = parser-topdown

CC = gcc
CFLAGS = -Werror -Wall -pedantic -std=c99

OBJS = $(PARSER).o $(SCANNER).o dstring.o tables.o
EXES = $(EXECUTABLE) $(SCAN_TEST_BIN)

all :  $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^

clean:
	rm -f *.o $(EXES)

.PHONY: all clean unit_tests
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -Wvla -pedantic
LDFLAGS=-fsanitize=address
LDLIBS=-lcriterion

OBJS=src/myfind.o src/lexer.o src/stack.o src/shunting_yard.o src/evaluate.o
BIN=myfind
TESTNAME=testsuite
TESTSUITE=tests/testsuite

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

check: $(OBJS) $(TESTSUITE).o
	$(CC) $(CFLAGS) -o $(TESTNAME) $^ $(LDFLAGS) $(LDLIBS)
	./$(TESTNAME)

clean:
	$(RM) tests/*.o src/*.o $(TESTNAME) $(BIN)
	$(RM) *.html *.css tests/*.gcno src/*.gcno tests/*.gcda src/*.gcda

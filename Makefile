CC = gcc
CFLAGS = -Wall -g
RM = rm -f
OBJS = sbcc.o scanner.o parser.o set.o
PROG = sbcc
TEST-OBJS = test-sbcc.o scanner.o parser.o set.o
TEST-PROG = test-sbcc

ALL: $(PROG) $(TEST-PROG)

sbcc.o: types.h scanner.h parser.h sbcc.c
scanner.o: types.h scanner.h scanner.c
parser.o: types.h parser.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(TEST-PROG): $(TEST-OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

test: $(TEST-PROG)
	./$(TEST-PROG)

clean:
	$(RM) $(PROG) $(OBJS) $(TEST-PROG) $(TEST-OBJS)

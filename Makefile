CC = gcc
CFLAGS = -Wall
RM = rm -f
OBJS = acc.o scanner.o parser.o
PROG = acc
TEST-OBJS = test-acc.o scanner.o parser.o
TEST-PROG = test-acc

ALL: $(PROG)

acc.o: types.h scanner.h parser.h
scanner.o: types.h scanner.h
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

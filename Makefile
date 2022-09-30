CC = gcc
CFLAGS = -Wall
RM = rm -f
OBJS = acc.o scanner.o parser.o
TARGET = acc

ALL: $(TARGET)

acc.o: types.h scanner.h parser.h
scanner.o: types.h scanner.h
parser.o: types.h parser.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(TARGET) $(OBJS)

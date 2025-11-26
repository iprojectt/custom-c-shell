# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g

OBJS = main.o parser.o builtins.o history.o execute.o signals.o

myshell: $(OBJS)
	$(CC) $(CFLAGS) -o myshell $(OBJS)

main.o: main.c headers.h
parser.o: parser.c headers.h
builtins.o: builtins.c headers.h
history.o: history.c headers.h
execute.o: execute.c headers.h
signals.o: signals.c headers.h

clean:
	rm -f *.o myshell

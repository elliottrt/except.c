
CFLAGS+=-std=c11 -Wall -Wextra -pedantic

main: main.c except.c except.h
	$(CC) main.c except.c -o main $(CFLAGS)


CFLAGS+=-std=c11 -Wall -Wextra -pedantic -Wsign-conversion -Wconversion -Wno-unused-function
# -Weverything -Wno-pre-c11-compat

main: main.c except.c except.h
	$(CC) main.c except.c -o main $(CFLAGS)

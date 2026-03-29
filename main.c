
#include <stdio.h>

#include "except.h"

static void always_throws(void) {
	throw(1, "always_throws");
}

#define NUM printf("except_num() = %u\n", _except_num())

static void single_main(void) {
	NUM;
	try {
		always_throws();

		printf("no exception :(\n");

		NUM;
	} catch(e) {
		printf("caught exception %d from %s:%u: %s\n", e->code, e->file, e->line, e->message);

		NUM;
	}
	NUM;
}

static void double_main(void) {
	NUM;
	try {
		try {
			printf("try-try\n");
			NUM;
		} catch(e) {
			printf("try-try-catch\n");
			NUM;
		}

		printf("try\n");
		NUM;
	} catch(e) {
		printf("try-catch\n");
		NUM;
	}
	NUM;
}

static void uncaught_main(void) {
	throw(69, "hello!");
}

int main(void) {
	try {
		throw(0, "bad!");
	} catch(e) {
		printf("e: %s\n", e->message);
	}
}

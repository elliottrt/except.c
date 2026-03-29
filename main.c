
#include <stdio.h>
#include <errno.h>

#include "except.h"

static void always_throws(void) {
	throw(1, "always_throws");
}

static void single_main(void) {
	try {
		always_throws();

		printf("no exception :(\n");
	} catch(e) {
		printf("caught exception %d from %s:%u: %s\n", e->code, e->file, e->line, e->message);
	}
}

static void double_main(void) {
	try {
		try {
			printf("try-try\n");
		} catch(e) {
			printf("try-try-catch\n");
		}

		printf("try\n");
	} catch(e) {
		printf("try-catch\n");
	}
}

static void uncaught_main(void) {
	throw(69, "hello!");
}

static void catch_specific_main(void) {
	try {
		throw(2, "exception!");
	} catch_code(e, 1, 3) {
		printf("caught a 1 or 3!\n");
	} catch_code(e, 2) {
		printf("caught a 2!\n");
	} catch(e) {
		printf("caught something else!\n");
	}
}

static void no_braces(void) {
	try throw(10, "look ma, no braces!");
	catch(e) printf("caught %d: %s at %s:%u\n", e->code, e->message, e->file, e->line);
}

static void throw_in_catch(void) {
	try {
		try {
			throw(1, "should see this again");
		} catch(e) {
			rethrow(e);
		}
	} catch(e) {
		printf("caught %d: %s from %s:%u\n", e->code, e->message, e->file, e->line);
	}
}

static void throws_errno(void) {
	const char *path = "/aifjdsifjsodaifjsdf.ahfsjdoifs";

	try {
		FILE *f = fopen(path, "p");
		if (!f) throw_errno(errno);
		else fclose(f);
	} catch(e) {
		printf("error: cannot open %s: %s\n", path, e->message);
	}
}

int main(void) {
	throws_errno();
}

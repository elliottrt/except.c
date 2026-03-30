
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

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
	} catch_case(e, 1, 3) {
		printf("caught a 1 or 3!\n");
	} catch_case(e, 2) {
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

/* note that exception values must be nonzero but can be negative */
enum { DIVISION_BY_ZERO_EXCEPTION = 1, INVALID_OP_EXCEPTION };

enum op { ADD, SUB, MUL, DIV };

static double do_op(enum op op, double lhs, double rhs) {
	// note that `return` is fine here. It's only immediately
	// within a `try` block that `try_return` is required.
	switch (op) {
	case ADD:
		return lhs + rhs;
	case SUB:
		return lhs - rhs;
	case MUL:
		return lhs * rhs;
	case DIV:
		if (rhs == 0.0)
			throw(DIVISION_BY_ZERO_EXCEPTION, "division by zero");
		return lhs / rhs;
	default:
		throw(INVALID_OP_EXCEPTION, "invalid operation %d", op);
	}
}

static void print_op(enum op op, double lhs, double rhs) {
	try {
		double value = do_op(op, lhs, rhs);
		printf("%f <%d> %f = %f\n", lhs, op, rhs, value);
	} catch (e) {
		printf(
			"error occurred during %f <%d> %f: %s\n",
			lhs, op, rhs, e->message
		);
	}
}

static void example1(void) {
	print_op(ADD, 1, 2);
	print_op(MUL, 10, 10);
	print_op(DIV, 1, 0);
	print_op(100, 5, 6);
}

int main(void) {
	example1();
}

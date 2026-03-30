# Except

Cross-platform exception handling (try-catch, throw) implemented in C11.
Exception codes, which are passed to throw and available from the catch or catch_code block, are user-defined but must be nonzero.

Important Notes:
- `return`, `goto`, or `break` within a try block prevents cleanup of
	internal resources so must be avoided.
- Uncaught exceptions are displayed to stderr and the program exits.
- Syntax highlighters can struggle with the macro expansions and display
	invalid brackets when they are in fact correct.

## Getting Started

Copy `except.c` and `except.h` into your project, `#include "except.h"` where needed, and compile `except.c` into your binary with the `-std=c11` flag.

## Contrived Example

```c
#include <stdio.h>
#include "except.h"

/* note that exception values must be nonzero but can be negative */
enum { DIVISION_BY_ZERO_EXCEPTION = 1, INVALID_OP_EXCEPTION };

enum op { ADD, SUB, MUL, DIV };

double do_op(enum op op, double lhs, double rhs) {
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

void print_op(enum op op, double lhs, double rhs) {
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

int main(void) {
	print_op(ADD, 1, 2);
	print_op(MUL, 10, 10);
	print_op(DIV, 1, 0);
	print_op(100, 5, 6);

	return 0;
}
```

Output:
```bash
$ cc example.c except.c && ./a.out
1.000000 <0> 2.000000 = 3.000000
10.000000 <2> 10.000000 = 100.000000
error occurred during 1.000000 <3> 0.000000: division by zero
error occurred during 5.000000 <100> 6.000000: invalid operation 100
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

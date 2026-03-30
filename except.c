
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "except.h"

typedef struct {
	Exception exception;
	jmp_buf try_location;
} _Except;

#define EXC_STACK_SIZE (64)

_Thread_local static _Except _except_stack[EXC_STACK_SIZE] = {0};
_Thread_local static unsigned _except_count = 0;

/* report_uncaught
	Inner helper for report_uncaught taking a va_list instead of ...
	Note: calls va_end() on ap.
*/
_Noreturn static void report_uncaughtv(
	int code, const char *file, unsigned line, const char *fmt, va_list ap
) {
	fprintf(stderr, "except: uncaught exception %d at %s:%u: ", code, file, line);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);

	exit(code);
}

/* report_uncaught
	If there is no try-catch block that will handle this exception,
	the program must exit.
*/
_Noreturn static void report_uncaught(
	int code, const char *file, unsigned line, const char *fmt, ...
) {
	va_list ap;
	va_start(ap, fmt);
	report_uncaughtv(code, file, line, fmt, ap);

	exit(code);
}

/* _except_throw
	Throw an exception given information about it. file and line should be set
	by the throw() macro. Aborts and prints message if the code is 0.
	If there is no try-catch block to handle this, reports an uncaught exception.
*/
_Noreturn void _except_throw(
	int code, const char *file, unsigned line, const char *fmt, ...
) {
	// make sure code isn't zero
	if (code == 0) {
		fprintf(stderr, "except: exception at %s:%u with zero exit code\n", file, line);
		abort();
	}

	va_list ap;
	va_start(ap, fmt);

	// if there are no try-catch blocks around this,
	// it is uncaught so we report error + exit.
	if (_except_count == 0) {
		report_uncaughtv(code, file, line, fmt, ap);
	}

	_Except *top = &_except_stack[_except_count - 1];

	top->exception.code = code;
	top->exception.file = file;
	top->exception.line = line;

	vsnprintf(top->exception.message, EXC_MSG_SIZE - 1, fmt, ap);
	va_end(ap);

	longjmp(top->try_location, 1);
}

/* _except_throw
	Throw an exception that was previously thrown by a catch or
	catch_case block. Updates its file path and line number.
*/
_Noreturn void _except_rethrow(
	const Exception *e, const char *file, unsigned line
) {
	// make sure that it was the last thrown exception.
	// this also accounts for a null pointer.
	if (e != &_except_stack[_except_count].exception) {
		fprintf(stderr, "except: invalid rethrow exception at %s:%u\n", file, line);
		abort();
	}

	// if there are no try-catch blocks around this,
	// it is uncaught so we report error + exit.
	if (_except_count == 0) {
		report_uncaught(e->code, file, line, "%s", e->message);
	}

	// copy its data into the empty exception currently at the stack top
	_Except *top = &_except_stack[_except_count - 1];

	// copy the rethrown exception into the current empty buffer
	memcpy(&top->exception, e, sizeof(top->exception));
	// change line and file
	top->exception.file = file;
	top->exception.line = line;

	longjmp(top->try_location, 1);
}

/* _except_errno
	Throws an exception with code=num and message=strerror(errno).
*/
_Noreturn void _except_errno(int num, const char *file, unsigned line) {
	_except_throw(num, file, line, "%s", strerror(num));
}

/* _except_push
	Pushes an _Except buffer onto the stack and returns its jmp_buf for use
	by the try macro.
*/
jmp_buf *_except_push(void) {
	assert(_except_count < EXC_STACK_SIZE);
	return &_except_stack[_except_count++].try_location;
}

/* _except_pop
	Removes the top exception and returns its value. This does mean that
	exceptions thrown during a catch block will overwrite the exception.
*/
Exception *_except_pop(void) {
	assert(_except_count > 0);
	return &_except_stack[--_except_count].exception;
}

/* _except_is
	Returns nonzero if the current exception is one of the argument codes.
	Due to C va_arg requirements, the first must be a named parameter.
*/
int _except_is(int next, ...) {
	va_list ap;
	va_start(ap, next);

	assert(_except_count > 0);
	int code = _except_stack[_except_count - 1].exception.code;

	do {
		if (code == next) return 1;
	} while ((next = va_arg(ap, int)) != 0);

	va_end(ap);
	return 0;
}

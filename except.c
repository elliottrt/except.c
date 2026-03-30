
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "except.h"

#define EXC_STACK_SIZE (64)

_Thread_local static jmp_buf _except_stack[EXC_STACK_SIZE] = {0};
_Thread_local static Exception _exception;
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

	_exception.code = code;
	_exception.file = file;
	_exception.line = line;

	vsnprintf(_exception.message, EXC_MSG_SIZE - 1, fmt, ap);
	va_end(ap);

	longjmp(_except_stack[_except_count - 1], 1);
}

/* _except_throw
	Throw an exception that was previously thrown by a catch or
	catch_case block. Updates its file path and line number.
*/
_Noreturn void _except_rethrow(const char *file, unsigned line) {
	// if there are no try-catch blocks around this,
	// it is uncaught so we report error + exit.
	if (_except_count == 0) {
		report_uncaught(_exception.code, file, line, "%s", _exception.message);
	}

	// change line and file
	_exception.file = file;
	_exception.line = line;

	longjmp(_except_stack[_except_count - 1], 1);
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
	if (_except_count >= EXC_STACK_SIZE) {
		fprintf(stderr, "except: exception stack overflow\n");
		abort();
	}
	return &_except_stack[_except_count++];
}

/* _except_pop
	Pops the exception destination from the stack from the parent try
	and return a pointer to the exception.
*/
const Exception *_except_pop(void) {
	if (_except_count == 0) {
		fprintf(stderr, "except: exception stack underflow\n");
		abort();
	}
	_except_count -= 1;
	return &_exception;
}

/* _except_is
	Returns nonzero if the current exception is one of the argument codes.
	Due to C va_arg requirements, the first must be a named parameter.
*/
int _except_is(int next, ...) {
	va_list ap;
	va_start(ap, next);

	assert(_except_count > 0);

	do {
		if (_exception.code == next) return 1;
	} while ((next = va_arg(ap, int)) != 0);

	va_end(ap);
	return 0;
}


#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "except.h"

typedef struct {
	Exception exc;
	jmp_buf buf;
} _Except;

#define EXC_STACK_SIZE (64)

_Thread_local static _Except _except_stack[EXC_STACK_SIZE] = {0};
_Thread_local static unsigned _except_count = 0;

_Noreturn static void report_uncaught(
	int code, const char *file, unsigned line, const char *fmt, va_list ap
) {
	fprintf(stderr, "except: uncaught exception %d at %s:%u: ", code, file, line);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);

	exit(code);
}

_Noreturn void _except_throw(
	int code, const char *file, unsigned line, const char *fmt, ...
) {
	// make sure code isn't zero, but if debug is off then force it.
	if (code == 0) {
		fprintf(stderr, "except: exception at %s:%u with zero exit code\n", file, line);
		abort();
	}

	va_list ap;
	va_start(ap, fmt);

	// if there are no try-catch blocks around this,
	// it is uncaught so we report error + exit.
	if (_except_count == 0)
		report_uncaught(code, file, line, fmt, ap);

	_Except *top = &_except_stack[_except_count - 1];

	top->exc.code = code;
	top->exc.file = file;
	top->exc.line = line;

	vsnprintf(top->exc.message, EXC_MSG_SIZE - 1, fmt, ap);
	va_end(ap);

	longjmp(top->buf, 1);
}

jmp_buf *_except_push(void) {
	assert(_except_count < EXC_STACK_SIZE);
	return &_except_stack[_except_count++].buf;
}

Exception *_except_pop(void) {
	assert(_except_count > 0);
	return &_except_stack[--_except_count].exc;
}

unsigned _except_num(void) {
	return _except_count;
}


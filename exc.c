
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "exc.h"

typedef struct {
	Exception exc;
	jmp_buf buf;
} _Exc;

#define EXC_STACK_SIZE (64)

_Thread_local static _Exc _exc_stack[EXC_STACK_SIZE] = {0};
_Thread_local static unsigned _exc_count = 0;

void report_uncaught(int code, const char *file, int line, const char *fmt, va_list ap) {
	fprintf(stderr, "exc: uncaught exception %d at %s:%d: ", code, file, line);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);

	exit(code ? code : 1);
}

void _exc_throw(int code, const char *file, int line, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	if (_exc_count == 0)
		report_uncaught(code, file, line, fmt, ap);

	_Exc *top = &_exc_stack[_exc_count - 1];

	top->exc.code = code;
	top->exc.file = file;
	top->exc.line = line;

	vsnprintf(top->exc.message, EXC_MSG_SIZE - 1, fmt, ap);
	va_end(ap);

	longjmp(top->buf, 1);
}

jmp_buf *_exc_push(void) {
	assert(_exc_count < EXC_STACK_SIZE);
	return &_exc_stack[_exc_count++].buf;
}

Exception *_exc_pop(void) {
	assert(_exc_count > 0);
	return &_exc_stack[--_exc_count].exc;
}

unsigned _exc_num(void) {
	return _exc_count;
}


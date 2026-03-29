/* exc.h
	Cross-platform exception handling (try-catch, throw) implemented in C.
	Exception codes, which are passed to throw and available from the catch
	block, are user-defined.

	Limitations: returning or goto'ing out of a try block prevents cleanup of
	internal resources, so it must be avoided.
*/

#ifndef _EXC_H_
#define _EXC_H_

#include <setjmp.h>

#define EXC_MSG_SIZE (256 - 2 * sizeof(int) - sizeof(char *))

/* type Exception
	All information is intended to be read by the user.
	code    --  integer code from throw(CODE, FMT, ...) to identify exception type.
	file    -- file path where the exception was thrown.
	line    -- line where the exception was thrown.
	message -- string from throw(CODE, FMT, ...) describing the cause.
*/
typedef struct {
  int code, line;
  const char *file;
  char message[EXC_MSG_SIZE];
} Exception;

/* throw(CODE, FMT, ...)
	Throw an exception with the given code and a message with arguments
	formatted as with printf. Also records the file and line location.
	CODE -- integer code that identifies the exception
	FMT  -- format string of the exception message
	...  -- arguments of the formatted exception message
*/
#define throw(CODE, ...) _exc_throw(CODE, __FILE__, __LINE__, __VA_ARGS__)

/* try
	Code structure keyword starting a try {} catch(NAME) {} block.
	Must be closed by catch(NAME).
	Do not return from or goto out of the block.
*/
#define try if (!setjmp(*_exc_push())) for (int _exc__flag = 0; _exc__flag++ == 0; _exc_pop())

/* catch(NAME)
	Catch an exception thrown within the above try block.
	Must follow a try block.
	Within the catch block, the exception may be referenced by the given name.
*/
#define catch(NAME) else for (const Exception *NAME = _exc_pop(); NAME; NAME = NULL)

// TODO: rethrow exception given pointer to it from catch
// TODO: catch specific code (or multiple) (should just be) changing else to else if (...) in #define catch

/*
	INTERNAL USE - DO NOT USE
*/

void _exc_throw(int, const char *, int, const char *, ...);
jmp_buf * _exc_push(void);
Exception *_exc_pop(void);

unsigned _exc_num(void);

#endif // _EXC_H_

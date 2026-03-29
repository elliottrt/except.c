/* except.h
	Cross-platform exception handling (try-catch, throw) implemented in C11+.
	Exception codes, which are passed to throw and available from the catch
	block, are user-defined but must be nonzero.

	Limitations: returning or goto'ing out of a try block prevents cleanup of
	internal resources, so it must be avoided.
*/

#ifndef _EXC_H_
#define _EXC_H_

#include <setjmp.h>

#define EXC_MSG_SIZE (256 - 2 * sizeof(int) - sizeof(char *))

/* type Exception
	All information is intended to be read by the user.
	code    -- nonzero integer code from throw to identify exception type.
	file    -- file path where the exception was thrown.
	line    -- line where the exception was thrown.
	message -- string from throw describing the cause.
*/
typedef struct {
  int code;
  unsigned line;
  const char *file;
  char message[EXC_MSG_SIZE];
} Exception;

/* throw(CODE, FMT, ...)
	Throw an exception with the given code and a message with arguments
	formatted as with printf. Also records the file and line location.
	CODE -- nonzero integer code that identifies the exception
	FMT  -- format string of the exception message
	...  -- arguments of the formatted exception message
*/
#define throw(CODE, ...) _except_throw(CODE, __FILE__, __LINE__, __VA_ARGS__)

/* try
	Code structure keyword starting a try {} catch(NAME) {} block.
	Must be closed by catch(NAME).
	Do not return from or goto out of the block.
*/
// TODO: use __LINE__ for unique id?
#define try if (!setjmp(*_except_push())) for (int _except_flag = 0; _except_flag++ == 0; _except_pop())

/* catch(NAME)
	Catch an exception thrown within the above try block.
	Must follow a try block.
	Within the catch block, the exception may be referenced by the given name.
*/
#define catch(NAME) else for (const Exception *NAME = _except_pop(); NAME; NAME = NULL)

// TODO: rethrow exception given pointer to it from catch
// TODO: catch specific code (or multiple) (should just be) changing else to else if (...) in #define catch
//		 make bool _exc_has(...) method and call with `else if (_exc_has(__VA_ARGS__, 0)) for ...`
// TODO: is it possible to record a stacktrace?

/*
	INTERNAL USE - DO NOT USE
*/

_Noreturn void _except_throw(int, const char *, unsigned, const char *, ...);
jmp_buf * _except_push(void);
Exception *_except_pop(void);

unsigned _except_num(void);

#endif // _EXC_H_

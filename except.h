/* except.h
	Cross-platform exception handling (try-catch-finally, throw) implemented in C11.
	Exception codes, which are passed to throw and available from the catch
	or catch_code block, are user-defined but must be nonzero.

	Important Notes:
	1. `return`, `goto`, or `break` within a try block prevents cleanup of
		internal resources so must be avoided.
	2. `throw` or similar within a catch or catch_code block prevents
		`finally` blocks from running.
	3. Uncaught exceptions are displayed to stderr and the program exits.
	4. Syntax highlighters can struggle with the macro expansions.
*/

#ifndef _EXC_H_
#define _EXC_H_

#include <setjmp.h>

#define EXC_MSG_SIZE (256)

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

/* throw(CODE, FMT, ARGS...)
	Throw an exception with the given code and a message with arguments
	formatted as though by printf. Also records the file and line location.
	CODE -- nonzero integer code that identifies the exception.
	FMT  -- format string of the exception message.
	...  -- arguments of the formatted exception message.
*/
#define throw(CODE, ...) _except_throw(CODE, __FILE__, __LINE__, __VA_ARGS__)

/* rethrow(EXCEPT)
	Rethrows an exception within a catch or catch_code block.
	EXCEPT -- pointer to an exception provided by catch or catch_code.
*/
#define rethrow(EXCEPT) _except_rethrow(EXCEPT, __FILE__, __LINE__)

/* throw_errno(ERRNO)
	Throws an exception with code=ERRNO and message=strerror(ERRNO).
	Often ERRNO=errno.
*/
#define throw_errno(ERRNO) _except_errno(ERRNO, __FILE__, __LINE__)

/* try
	Code structure keyword starting a try {} catch(NAME) {} block.
	Must be closed by catch(NAME).
	Do not return from or goto out of the block.
*/
#define try _except_try(__LINE__)

/* catch(NAME)
	Catch an exception thrown within the above try block.
	Must follow a try block or catch_code block.
	Within the catch block, the exception may be referenced by NAME.
	If an exception is thrown within the catch block, NAME becomes invalid.
*/
#define catch(NAME) _except_catch(NAME)

/* catch_code(NAME, CODES...)
	Catch an exception matching one of CODES thrown within the above try block.
	Must follow a try block or catch_code block.
	Within the catch_code block, the exception may be referenced by NAME.
	If an exception is thrown within the catch_code block, NAME becomes invalid.
*/
#define catch_code(NAME, ...) _except_catch_code(NAME, __VA_ARGS__)

/* finally
	This block should be placed at the end of a try-catch group.
	The code in it will run as long as control flow reaches the block.
	If there are multiple finally blocks, each will run.
*/
#define finally _except_finally

// TODO: consider continue; statement for the try,catch,catch_code block
//			because it forces cleanup then exits the block, could be useful
// TODO: namespacing for all functions by default, disableable with header flag
// TODO: record rethrow locations to display
// TODO: rethrow_as(CODE, ...) that changes code and message
// TODO: better name for catch_code
// TODO: special return/goto/break within try block that cleans up the exception
//			or maybe a special function/macro that does the cleanup
//			all we need is to call _except_pop()
// TODO: see DbgHelp.h (windows) and execinfo.h (glibc)

/*
	INTERNAL USE - DO NOT USE
*/

#define _except_try(ID) _except_try1(ID)
#define _except_try1(ID) if (!setjmp(*_except_push())) \
	for (int _except_##ID=1;_except_##ID--;_except_pop())

#define _except_catch(NAME) else \
	for (const Exception *NAME = _except_pop(); NAME; NAME = NULL)

#define _except_catch_code(NAME, ...) else if (_except_is(__VA_ARGS__, 0)) \
	for (const Exception *NAME = _except_pop(); NAME; NAME = NULL)

#define _except_finally

_Noreturn void _except_throw(int, const char *, unsigned, const char *, ...);
_Noreturn void _except_rethrow(const Exception *, const char *, unsigned);
_Noreturn void _except_errno(int, const char *, unsigned);
jmp_buf *_except_push(void);
Exception *_except_pop(void);
int _except_is(int, ...);

#endif // _EXC_H_

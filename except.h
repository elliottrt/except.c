/* except.h
	Cross-platform exception handling (try-catch, throw) implemented in C11.
	Exception codes, which are passed to throw and available from the catch
	or catch_case block, are user-defined but must be nonzero.

	Important Notes:
	1. `return`, `goto` within a try block prevents cleanup of internal resources.
		Use `try_return` or `try_goto` while in a try block instead. This is only in the immediate
		scope of the block, functions called from a try block should use the regular keywords.
	2. Uncaught exceptions are displayed to stderr and the program exits.
	3. Syntax highlighters can struggle with the macro expansions.
	4. There is no `finally` block because there is currently no known way to
		implement it so that it _always_ runs.
	5. Locations of errors are enabled by default and can be disabled by
		defining `EXCEPT_NO_LOCATION`.
	6. `try` (with or without `catch_case`) not followed by `catch` will ignore
		errors that are not caught. This is usually dangerous and should be avoided.
*/

#ifndef _EXCEPT_H_
#define _EXCEPT_H_

#include <setjmp.h>

#define EXC_MSG_SIZE (256)

/* type Exception
	All information is intended to be read by the user.
	code    -- nonzero integer code from throw to identify exception type.
	file    -- file path where the exception was thrown.
	line    -- line where the exception was thrown.
	func    -- function where the exception was thrown.
	message -- string from throw describing the cause.
*/
typedef struct {
	int code;
	unsigned line;
	const char *file;
	const char *func;
	char message[EXC_MSG_SIZE];
} Exception;

/* throw(CODE, FMT, ARGS...)
	Throw an exception with the given code and a message with arguments
	formatted as though by printf. Also records the file and line location.
	If there is no try block that catches it, calls the exception handler.
	CODE -- nonzero integer code that identifies the exception.
	FMT  -- format string of the exception message.
	...  -- arguments of the formatted exception message.
*/
#define throw(CODE, ...) _except_throw(CODE, _except_location, __VA_ARGS__)

/* rethrow
	Rethrows the exception within a catch or catch_case block.
*/
#define rethrow _except_rethrow(_except_location)

/* throw_errno(ERRNO)
	Throws an exception with code=ERRNO and message=strerror(ERRNO).
	Often this will have ERRNO=errno.
*/
#define throw_errno(ERRNO) _except_errno(ERRNO, _except_location)

/* try
	Code structure keyword starting a try {} catch(NAME) {} block.
	Must be closed by catch(NAME).
	Do not return from or goto out of the block.
*/
#define try _except_try(__LINE__)

/* catch(NAME)
	Catch an exception thrown within the above try block.
	Must follow a try block or catch_case block.
	Within the catch block, the exception may be referenced by NAME.
	If an exception is thrown within the catch block, NAME becomes invalid.
*/
#define catch(NAME) _except_catch(NAME)

/* catch_case(NAME, CODES...)
	Catch an exception matching one of CODES thrown within the above try block.
	Must follow a try block or catch_case block.
	Within the catch_case block, the exception may be referenced by NAME.
	If an exception is thrown within the catch_case block, NAME becomes invalid.
*/
#define catch_case(NAME, ...) _except_catch_case(NAME, __VA_ARGS__)

/* try_return(VALUE?)
	Return a value from within a `try` block safely.
	May be called with 0 or 1 arguments.
	Use this instead of `return` within a `try` block.
*/
#define try_return(VALUE) _except_wrap(return, VALUE)

/* try_goto(LABEL)
	goto a label from within a `try` block safely.
	Use this instead of `goto` within a `try` block.
*/
#define try_goto(LABEL) _except_wrap(goto, VALUE)

/* except_handler(handler)
	Sets the uncaught exception handler, which is called when an exception
	has no try-catch block to go to. If the handler does not exit the program
	itself or is NULL, exit(EXIT_FAILURE) will be called.
*/
void except_handler(void (*handler)(const Exception *));

// TODO: see DbgHelp.h (windows) and execinfo.h (glibc) for stacktrace

/*
	INTERNAL - DO NOT USE
*/

#ifdef EXCEPT_NO_LOCATION
	#define _except_location "<none>", 0, "<none>"
#else
	#define _except_location __FILE__, __LINE__, __func__
#endif

#define _except_try(ID) _except_try1(ID)
#define _except_try1(ID) if (!setjmp(*_except_push()) || (_except_pop(), 0)) \
	for (int _except_##ID=1;_except_##ID--;_except_pop())
#define _except_catch(NAME) else \
	for (const Exception *NAME = _except_get(); NAME; NAME = NULL)
#define _except_catch_case(NAME, ...) else if (_except_is(__VA_ARGS__, 0)) \
	for (const Exception *NAME = _except_get(); NAME; NAME = NULL)
#define _except_wrap(K, X) do { _except_pop(); K X; } while (0)
_Noreturn void _except_throw(int, const char *, unsigned, const char *, const char *, ...);
_Noreturn void _except_rethrow(const char *, unsigned, const char *);
_Noreturn void _except_errno(int, const char *, unsigned, const char *);
jmp_buf *_except_push(void);
void _except_pop(void);
int _except_is(int, ...);
const Exception *_except_get(void);

#endif // _EXCEPT_H_

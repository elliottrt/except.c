#ifndef _EXC_H_
#define _EXC_H_

#include <setjmp.h>

#define EXC_MSG_SIZE (256 - 2 * sizeof(int) - sizeof(char *))

typedef struct {
  int code, line;
  const char *file;
  char message[EXC_MSG_SIZE];
} Exception;

#define throw(CODE, ...) _exc_throw(CODE, __FILE__, __LINE__, __VA_ARGS__)
#define try if (!setjmp(*_exc_push())) for (int _exc__flag = 0; _exc__flag++ == 0; _exc_pop())
#define catch(NAME) else for (const Exception *NAME = _exc_pop(); NAME; NAME = NULL)

// TODO: rethrow exception given pointer to it from catch
// TODO: catch specific code (or multiple) (should just be) changing else to else if (...) in #define catch

void _exc_throw(int, const char *, int, const char *, ...);
jmp_buf * _exc_push(void);
Exception *_exc_pop(void);

unsigned _exc_num(void);

#endif // _EXC_H_

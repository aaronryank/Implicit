#ifndef _SIMPLESTACK_H
#define _SIMPLESTACK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <sys/stat.h>

enum {
  CP_SBCS = 1,
  CP_UTF8 = 2
} codepages;

enum {
  ARGUMENT = 1,
  COMMAND  = 2
} types_in_source;

enum {
  TYPE_INT,
  TYPE_STR,
  TYPE_FLT
} stack_value_types;

#define STACK_STRING_SIZE 1000
#define INPUT_BUFFER_SIZE 1000

#define zero(x) memset(&x,0,sizeof(x));
#define ssprintf(...) flags.cn ? append_to_outbuf(__VA_ARGS__) : printf(__VA_ARGS__)
#define ssputchar(x) flags.cn ? append_to_outbuf("%c",x) : putchar(x)

int wcstoi(wint_t*);
int append_to_outbuf(const char *,...);
int getcommand(wint_t*);
int execute(wint_t*,int);
void do_skip(int);
int do_jump(void);
int implicit_input(int,int);
void debug(wint_t*,int);
void stack_realloc(void);
int coolstuff(void);
#ifndef _WIN32
char *itoa(int,char*,int);
#endif

struct _stack {
  int val;
  char val_str[STACK_STRING_SIZE];
  float val_flt;
  int type;
};

struct _jump {
  long int pos;
  int val;
};

struct _flags {
  int cp; // codepage
  int db; // debugging
  int in; // interactive (requires debugging)
  int nn; // no newline
  int cn; // canvas mode
};

extern struct _stack *stack, mem;
extern struct _jump jumps[1000];
extern struct _flags flags;
extern int top, cur_stack_size;
extern int jumped, jumpnum;
extern int noprint, skip;
extern FILE *in;
// canvas mode
extern char *outbuf;
extern int cur_output_size;

#endif // simplestack.h
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <locale.h>

#define STACK_STRING_SIZE 1000
#define INPUT_BUFFER_SIZE 1000

enum {
  CP_SBCS = 1,
  CP_UTF8 = 2
} codepages;

struct {
  int cp; // codepage
  int db; // debugging
} flags;

FILE *in;

enum {
  ARGUMENT = 1,
  COMMAND  = 2
} types_in_source;

int wcstoi(wint_t*);

int getcommand(wint_t*);
int execute(wint_t*,int);
void do_skip(void);
int do_jump(void);
int implicit_input(int,int);
void debug(wint_t*,int);
#ifndef _WIN32
char *itoa(int,char*,int);
#endif

struct _stack {
  int val;
  char val_str[STACK_STRING_SIZE];
  float val_flt;
  int type;
} *stack, mem;

int top, cur_stack_size;

enum {
  TYPE_INT,
  TYPE_STR,
  TYPE_FLT
} stack_value_types;

int noprint;

struct _jump {
  long int pos;
  int val;
} jumps[1000];
int jumpnum, jumped;

#define zero(x) memset(&x,0,sizeof(x));

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"C.UTF-8");
    setlocale(LC_CTYPE,"C.UTF-8");

    if (argc != 3 || strchr(argv[1],'h')) {
        fprintf(stderr,"Usage: %s <arguments> <filename>\n",argv[0]);
        return 1;
    }

    int i, n = strlen(argv[1]);
    for (i = 0; i < n; i++) {
        switch(argv[1][i]) {
          case 's': flags.cp = CP_SBCS; break;
          case 'u': flags.cp = CP_UTF8; break;
          case 'd': flags.db = 1;       break;
        }
    }

    in = fopen(argv[2],"r");

    if (!in) {
        fprintf(stderr,"Error %d: Cannot open file %s: %s\n",errno,argv[2],strerror(errno));
        return 2;
    }

    stack = malloc(1000*sizeof(struct _stack));
    memset(stack,0,1000*sizeof(struct _stack));
    cur_stack_size = 1000;

    int loop = 1, args = 0;
    wint_t buf[INPUT_BUFFER_SIZE] = {(wint_t)0};
    wint_t command[INPUT_BUFFER_SIZE] = {(wint_t)0};

    while (loop)
    {
        int type = getcommand(buf);

        if (type == ARGUMENT) {
            loop = !!loop + execute(command,wcstoi(buf));
            args++;
        }
        else if (type == COMMAND) {
            loop = !!wcscmp(buf,L"EOF");   // truncate to bool

            if (!args) {
                int retval = execute(command,-1);
                if (retval == -1)
                    loop = 0;
                else
                    loop = loop || retval;
            }

            args = 0;

            /* this is crappy hardcoding but there's not a better way to do it */
            if (!jumped) {
                wcscpy(command,buf);
            }
            else {
                jumped = 0;
                wcscpy(command,L" ");
            }
        }
    }

    if (top && !noprint) {
        if (stack[top].type == TYPE_INT)
            printf("%d",stack[top].val);
        else if (stack[top].type == TYPE_STR)
            printf("%s",stack[top].val_str);
        else if (stack[top].type == TYPE_FLT)
            printf("%f",stack[top].val_flt);
    }

    fflush(stdout);
    fclose(in);
}

int getcommand(wint_t *buf)
{
    memset(buf,0,INPUT_BUFFER_SIZE*sizeof(wint_t));

    wint_t c;
    int s = 0;
    while (s < INPUT_BUFFER_SIZE-1)
    {
        c = getwc(in);

        if (isdigit(wctob(c))) {
            buf[s++] = c;
            while (isdigit(wctob((c = getwc(in)))))
                buf[s++] = c;
            ungetwc(c,in);
            return ARGUMENT;
        }
        else if (c == WEOF) {
            wcscpy(buf,L"EOF");
            return COMMAND;
        }
        else if (c == 0xAB) {                               // «
            buf[s++] = c;
            while ((c = getwc(in)) != 0xBB && c != WEOF)    // »
                buf[s++] = c;
            return COMMAND;
        }
        else if (c == 0xB0) { // °
            char tmp[100] = {0};
            itoa(stack[top].val,tmp,10);
            mbstowcs(buf,tmp,100);
            return ARGUMENT;
        }
        else {
            buf[s] = c;
            return COMMAND;
        }
    }

    memset(buf,0,INPUT_BUFFER_SIZE*sizeof(wint_t));
    return COMMAND;
}

/* return value:

   1  continue loop unconditionally
   0  exit loop if EOF reached
   -1 exit loop unconditionally
*/

int execute(wint_t *command, int args)
{
    int orig_args = args; // for debugging, some commands modify args

    if (!wcscmp(command,L"EOF")) {
        if (jumpnum) {
            jumps[jumpnum].val = -1;
            return do_jump();
        }
        return 0;
    }
    else if (command[0] == L'+') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (top > 1 && args == -1 && (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT))
        {
            stack[top-1].val += stack[top].val;
            stack[top-1].val_flt += stack[top].val_flt;
            zero(stack[top]);
            top--;
        }
        else if (stack[top-1].type == TYPE_STR)
        {
            if (stack[top].type != TYPE_STR)
                stack[top].val_str[0] = stack[top].val;

            args = abs(args);
            while (args--)
                strcat(stack[top-1].val_str,stack[top].val_str);

            zero(stack[top]);
            top--;
        }
        else if (stack[top].type == TYPE_INT || stack[top].type == TYPE_FLT)
        {
            stack[top].val += abs(args);
            stack[top].val_flt += abs(args);
        }
    }
    else if (command[0] == L'.') {
        if (!top && args == -1)
            implicit_input(1,TYPE_INT);
        else if (!top && args != -1)
            implicit_input(1,TYPE_STR);

        if (stack[top].type == TYPE_INT && args == -1)
            stack[top].val++;
        else if (stack[top].type == TYPE_STR && args != -1) {
            int i, l = strlen(stack[top].val_str);

            if (args >= l)
                return 1;

            char s[STACK_STRING_SIZE] = {0};
            strcpy(s,stack[top].val_str);
            memset(stack[top].val_str,0,STACK_STRING_SIZE);

            for (i = 0; l > 0; i++) {
                stack[top+i].type = TYPE_STR;
                strncpy(stack[top+i].val_str,s,args);

                strncpy(s,&s[(i+1)*args],args);

                l -= args;
            }

            top += (i - 1); // dafuq?
        }
    }
    else if (command[0] == L'-') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (top > 1 && args == -1 && (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT))
        {
            if (args == -1 && top > 1) {
                stack[top-1].val -= stack[top].val;
                stack[top-1].val_flt -= stack[top].val_flt;
                zero(stack[top]);
                top--;
            }
            else {
                stack[top].val -= abs(args);
                stack[top].val_flt -= (float) abs(args);
            }
        }
        else if (args && (stack[top].type == TYPE_INT || stack[top].type == TYPE_FLT))
        {
            stack[top].val -= abs(args);
            stack[top].val_flt -= abs(args);
        }
        else if (stack[top].type == TYPE_STR)
        {
            args = abs(args);
            while (args--)
                stack[top].val_str[strlen(stack[top].val_str)-1] = 0;
        }
    }
    else if (command[0] == L'*') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (top > 1 && args == -1 && (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT))
        {
            if (args == -1 && top > 1) {
                stack[top-1].val *= stack[top].val;
                stack[top-1].val_flt *= stack[top].val_flt;
                zero(stack[top]);
                top--;
            }
            else {
                stack[top].val *= abs(args);
                stack[top].val_flt *= (float) abs(args);
            }
        }
        else if (stack[top-1].type == TYPE_STR)
        {
            if (args == -1)
                args = 0;

            if (stack[top].type == TYPE_INT)
                stack[top].val_str[0] = stack[top].val;

            int sz = strlen(stack[top].val_str);

            /* make space in the string for the characters */
            char *t1 = malloc(strlen(&stack[top-1].val_str[args]) * sizeof(char));
            strcpy(t1,&stack[top-1].val_str[args]);
            strcpy(&stack[top-1].val_str[args+sz],t1);
            free(t1);

            /* add the characters */
            int i;
            for (i = 0; i < sz; i++)
                stack[top-1].val_str[args+i] = stack[top].val_str[i];

            zero(stack[top]);
            top--;
        }
        else if ((stack[top].type == TYPE_INT || stack[top].type == TYPE_FLT) && args != -1)
        {
            stack[top].val *= args;
            stack[top].val_flt *= args;
        }
    }
    else if (command[0] == L'/') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (top > 1 && args == -1 && (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT))
        {
            if (args == -1 && top > 1) {
                stack[top-1].val /= stack[top].val;
                stack[top-1].val_flt /= stack[top].val_flt;
                zero(stack[top]);
                top--;
            }
            else {
                stack[top].val /= abs(args);
                stack[top].val_flt /= abs(args);
            }
        }
        else if ((stack[top].type == TYPE_INT || stack[top].type == TYPE_FLT) && args != -1)
        {
            args && (stack[top].val /= args);
            args && (stack[top].val_flt /= args);
        }
        else if (stack[top-1].type == TYPE_STR)
        {
            if (args == -1)
                args = 0;

            if (stack[top].type == TYPE_INT)
                stack[top].val_str[0] = stack[top].val;

            int sz = strlen(stack[top].val_str);

            /* add the characters */
            int i;
            for (i = 0; i < sz; i++)
                stack[top-1].val_str[args+i] = stack[top].val_str[i];

            zero(stack[top]);
            top--;
        }
    }
    else if (command[0] == L'_') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (top > 1 && stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT && args == -1)
        {
            stack[top-1].val %= stack[top].val;
            zero(stack[top]);
            top--;
        }
        else if (stack[top].type == TYPE_INT && args != -1)
        {
            stack[top].val %= args;
        }
        else if (stack[top].type == TYPE_STR) {
            if (args == -1)
                args = strlen(stack[top].val_str)-1;

            stack[top+1].val = stack[top].val_str[args];
            top++;
        }
    }
    else if (command[0] == L'%') {
        if (args == -1) {
            if (stack[top].type == TYPE_INT)
                printf("%d",stack[top].val);
            else if (stack[top].type == TYPE_STR)
                printf("%s",stack[top].val_str);
            else if (stack[top].type == TYPE_FLT)
                printf("%f",stack[top].val_flt);
        }
        else
            printf("%d",args);
    }
    else if (command[0] == L'<') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (args == -1)
        {
            float f1 = (stack[top-1].type == TYPE_FLT) ? stack[top-1].val_flt : (stack[top-1].type == TYPE_STR) ? strlen(stack[top-1].val_str) : stack[top-1].val;
            float f2 = (stack[top].type == TYPE_FLT)   ? stack[top].val_flt   : (stack[top].type == TYPE_STR)   ? strlen(stack[top].val_str)   : stack[top].val;

            stack[top+1].val = (f1 < f2);
        }
        else {
            if (stack[top].type == TYPE_INT)
                stack[top+1].val = (stack[top].val < args);
            else if (stack[top].type == TYPE_FLT)
                stack[top+1].val = (stack[top].val_flt < args);
            else if (stack[top].type == TYPE_STR)
                stack[top+1].val = (strlen(stack[top].val_str) < args);
        }
        top++;
    }
    else if (command[0] == L'>') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (args == -1)
        {
            float f1 = (stack[top-1].type == TYPE_FLT) ? stack[top-1].val_flt : (stack[top-1].type == TYPE_STR) ? strlen(stack[top-1].val_str) : stack[top-1].val;
            float f2 = (stack[top].type == TYPE_FLT)   ? stack[top].val_flt   : (stack[top].type == TYPE_STR)   ? strlen(stack[top].val_str)   : stack[top].val;

            stack[top+1].val = (f1 > f2);
        }
        else {
            if (stack[top].type == TYPE_INT)
                stack[top+1].val = (stack[top].val > args);
            else if (stack[top].type == TYPE_FLT)
                stack[top+1].val = (stack[top].val_flt > args);
            else if (stack[top].type == TYPE_STR)
                stack[top+1].val = (strlen(stack[top].val_str) > args);
        }
        top++;
    }
    else if (command[0] == L'@') {
        if (args == -1)
            printf("%c",stack[top].val % 128);
        else
            printf("%c",args % 128);
    }
    else if (command[0] == L'&') {
        if (args) {
            noprint = 1;
            return -1;
        }
    }
    else if (command[0] == L'~') {
        args = abs(args);
        while (args--)
            stack[++top].val = getchar();
    }
    else if (command[0] == L'$') {
        args = abs(args);
        while (args--)
            scanf("%d",&stack[++top].val);  // int type by default
    }
    else if (command[0] == L'\'') {
        args = abs(args);
        while (args--)
        {
            fgets(stack[++top].val_str,999,stdin);
            if (stack[top].val_str[strlen(stack[top].val_str)-1] == '\n')
                stack[top].val_str[strlen(stack[top].val_str)-1] = 0;
            stack[top].type = TYPE_STR;
        }
    }
    else if (command[0] == L':') {
        if (args == -1 && !top)
            implicit_input(1,TYPE_INT);

        if (args == -1) {
            stack[top+1].val = stack[top].val;
            stack[top+1].val_flt = stack[top].val_flt;
            stack[top+1].type = stack[top].type;
            strcpy(stack[top+1].val_str,stack[top].val_str);
            top++;
        }
        else
            stack[++top].val = args; // int type by default
    }
    else if (command[0] == L';') {
        args = abs(args);
        while (args-- && top) {
            zero(stack[top]);
            top--;
        }
    }
    else if (command[0] == L'\\') {
        if (!top)
            implicit_input(1,TYPE_STR);

        if (args == -1) {
            if (stack[top].type == TYPE_INT)
                stack[top].val = -stack[top].val;
            else if (stack[top].type == TYPE_FLT)
                stack[top].val_flt = -stack[top].val_flt;
            else {
                int i, j = strlen(stack[top].val_str);
                char *dest = strdup(stack[top].val_str);

                for (i = 0; i < j; i++)
                    dest[i] = stack[top].val_str[j-i-1];  // -1 because j is 1-indexed
                dest[i] = 0; // dafuq?

                strcpy(stack[top].val_str,dest);
                free(dest);
            }
        }
        else {
            if (stack[top].type == TYPE_STR && args) {
                int l = strlen(stack[top].val_str);
                char *s = strdup(stack[top].val_str);
                memset(stack[top].val_str,0,STACK_STRING_SIZE);
                strcpy(stack[top].val_str,&s[args]);
                free(s);
            }
        }
    }
    else if (command[0] == L'^') {
        if (top == 0)
            implicit_input(1,TYPE_INT);

        if (stack[top].type == TYPE_INT) {
            args = (args == -1) ? 2 : args;
            while (--args)
                stack[top].val *= stack[top].val;
        }
        else if (stack[top].type == TYPE_FLT) {
            args = (args == -1) ? 2 : args;
            while (--args)
                stack[top].val_flt *= stack[top].val_flt;
        }
        else if (stack[top].type == TYPE_STR) {
            if (args == -1) {
                stack[top+1].val = strlen(stack[top].val_str);
                top++;
            }
            else
                memset(&stack[top].val_str[args],0,STACK_STRING_SIZE - args - 1);
        }
    }
    else if (command[0] == 0xAB) { // «
        args = abs(args);
        while (args--)
        {
            wcstombs(stack[++top].val_str,&command[1],INPUT_BUFFER_SIZE);
            stack[top].type = TYPE_STR;
        }
    }
    else if (command[0] == 0xA6) { // ¦
        if (!top)
            implicit_input(1,TYPE_STR);

        if (stack[top].type == TYPE_INT) {
            if (args == -1) {
                if (isupper(stack[top].val))
                    stack[top].val = tolower(stack[top].val);
                else
                    stack[top].val = toupper(stack[top].val);
            }
            else if (args == 1)
                stack[top].val = tolower(stack[top].val);
            else if (args == 2)
                stack[top].val = toupper(stack[top].val);
        }
        else if (stack[top].type == TYPE_STR) {
            int i;
            if (args == -1)
            {
                int l = strlen(stack[top].val_str);
                for (i = 0; i < l; i++) {
                    if (isupper(stack[top].val_str[i]))
                        stack[top].val_str[i] = tolower(stack[top].val_str[i]);
                    else if (islower(stack[top].val_str[i]))
                        stack[top].val_str[i] = toupper(stack[top].val_str[i]);
                }
            }
            else if (args == 1)
                for (i = 0; stack[top].val_str[i]; i++)
                    stack[top].val_str[i] = tolower(stack[top].val_str[i]);
            else if (args == 2)
                for (i = 0; stack[top].val_str[i]; i++)
                    stack[top].val_str[i] = toupper(stack[top].val_str[i]);
        }
    }
    else if (isalpha(wctob(command[0]))) {
        args = abs(args);
        while (args--)
            stack[++top].val = command[0];
    }
    else if (command[0] == L'\"') {
        int start;
        if (args == -1)
            start = 0;
        else
            start = top - args;

        struct _stack tmp = {0};
        int i, j;
        for (i = start + 1, j = 0; i <= top; i++) {
            if (stack[i].type == TYPE_INT) {
                tmp.val_str[j++] = stack[i].val;
            }
            else if (stack[i].type == TYPE_STR) {
                strcat(tmp.val_str,stack[i].val_str);
                j += strlen(stack[i].val_str);
            }
        }

        tmp.type = TYPE_STR;
        while (top && args) {
            zero(stack[top]);
            top--, args--;
        }
        memcpy(&stack[++top],&tmp,sizeof(struct _stack));
    }
    else if (command[0] == L'#') {
        if (args == -1) {
            stack[top+1].val = top;
            top++;
        }
        else {
            while (top > args) {
                zero(stack[top]);
                top--;
            }
        }
    }
    else if (command[0] == L'=') {
        if (top == 0)
            implicit_input(2,TYPE_INT);

        if (args == -1)
        {
            float f1 = (stack[top-1].type == TYPE_FLT) ? stack[top-1].val_flt : stack[top-1].val;
            float f2 = (stack[top].type == TYPE_FLT) ? stack[top].val_flt : stack[top].val;

            if (stack[top-1].type == TYPE_STR)
                stack[top+1].val = !strcmp(stack[top-1].val_str,stack[top].val_str);
            else
                stack[top+1].val = (f1 == f2);
        }
        else {
            stack[top+1].val = (stack[top].val == args);
        }
        top++;
    }
    else if (command[0] == L',') {
        if (args == -1)
            args = top-1;
        else
            args++;  // to avoid swapping with 0th stack index

        struct _stack tmp;
        memcpy(&tmp,&stack[args],sizeof(struct _stack));
        memcpy(&stack[args],&stack[top],sizeof(struct _stack));
        memcpy(&stack[top],&tmp,sizeof(struct _stack));
    }
    else if (command[0] == L'[') {
        memcpy(&mem,&stack[top],sizeof(struct _stack));
        zero(stack[top]);
        top--;
    }
    else if (command[0] == L']') {
        memcpy(&stack[++top],&mem,sizeof(struct _stack));
    }
    else if (command[0] == 0xA4) {
        //builtin(args);
    }
    else if (command[0] == L'?') {
        if (args == -1)
            args = stack[top].val;

        if (!args)
            do_skip();
    }
    else if (command[0] == L'!') {
        if (args == -1)
            args = stack[top].val;

        if (args)
            do_skip();
    }
    else if (command[0] == L'(') {
        if (args == -1) {
            jumps[jumpnum].val = -1;
            jumps[jumpnum].pos = ftell(in) - 1;
            jumpnum++;
        }
        else {
            int j = stack[args+1].val;
            if (!j) {
                int parens = 1;
                wint_t c = 0;
                while (parens && c != WEOF) {
                    c = getwc(in);
                    if (c == L'(')
                        parens++;
                    else if (c == L')')
                        parens--;
                }
            }
            else {
                jumps[jumpnum].val = args+1;
                jumps[jumpnum].pos = ftell(in) - 1;
                jumpnum++;
            }
        }
    }
    else if (command[0] == L')') {
        if (args == -1) {
            jumps[jumpnum].val = top;
            return do_jump();
        }
        else {
            jumps[jumpnum].val = args+1;
            return do_jump();
        }
    }
    else if (command[0] == 0xB6) { // ¶
        jumps[jumpnum].val = -1;
        return do_jump();
    }
    else if (command[0] == 0xC0) {
        strcpy(stack[++top].val_str,"abcdefghijklmnopqrstuvwxyz");
        stack[top].type = TYPE_STR;
    }
    else if (command[0] == L'|') {
        int i;
        for (i = 1; i <= top; i++)
            memcpy(&stack[i+top],&stack[i],sizeof(struct _stack));
        top += top;
    }
    else if (command[0] == 0xB8) { // cedilla ¸
        if (!top)
            implicit_input(1,TYPE_STR);

        if (stack[top].type == TYPE_STR && args == -1) {
            int i, l = strlen(stack[top].val_str);
            for (i = 0; i < l; i++)
                stack[top+i+1].val = stack[top].val_str[i];
            zero(stack[top]);
            top += l - 1;
        }
        else if (stack[top].type == TYPE_INT && args == -1) {
            stack[top].val--;
        }
    }
    else if (command[0] == 0xE9) { // é
        if (!top)
            implicit_input(1,TYPE_STR);

        if (stack[top].type == TYPE_STR) {
            int i, l = strlen(stack[top].val_str);
            for (i = 0; i < l; i++)
                stack[top].val_str[i] += abs(args);
        }
        else if (stack[top].type == TYPE_INT && args == -1) {
            if (stack[top].val)
                return -1;
        }
    }
    else if (command[0] == 0xE8) { // è
        if (!top)
            implicit_input(1,TYPE_STR);

        if (stack[top].type == TYPE_STR && args) {
            int i, l = strlen(stack[top].val_str);
            for (i = 0; i < l; i++)
                stack[top].val_str[i] -= abs(args);
        }
        else if (stack[top].type == TYPE_INT && args == -1) {
            if (stack[top].val) {
                noprint = 1;
                return -1;
            }
        }
    }
    else if (command[0] == 0xA7) { // §
        args = abs(args);

        if (stack[top].type == TYPE_STR) {
            char *s = strdup(stack[top].val_str);
            memset(stack[top].val_str,0,strlen(stack[top].val_str));

            while (args--)
                strcat(stack[top].val_str,s);

            stack[top].type = TYPE_STR;
        }
        else if (stack[top].type == TYPE_INT) {
            int i = 0;
            while (args--)
                stack[top].val_str[i++] = stack[top].val;

            stack[top].val = 0;
            stack[top].type = TYPE_STR;
        }
    }
    else if (command[0] == 0xB1) { // ±
        if (!top)
            implicit_input(1,TYPE_INT);

        if (args == -1 && stack[top].type == TYPE_INT) {
            stack[top+1].val = stack[top].val < 0 ? -1 : stack[top].val == 0 ? 0 : 1;
            top++;
        }
        else if (args != -1) {
            stack[++top].val = -args;
        }
    }
    else if (command[0] == 0xA1) { // ¡
        if (!top && args == -1)
            implicit_input(1,TYPE_INT);

        if (stack[top].type == TYPE_INT) {
            if (args == -1)
                args = stack[top].val;

            int i, n = args;
            for (i = 0; i < n; i++)
                stack[top+i].val = i+1;
            top += (i - 1);
        }
    }
    else if (command[0] == 0xDE) { // Þ
        if (!top && args == -1)
            implicit_input(1,TYPE_INT);

        if (args == -1)
            args = top;

        int sum = 0;
        while (args--) {
            sum += stack[top].val;
            zero(stack[top]);
            top--;
        }
        stack[++top].val = sum;
    }
    else if (command[0] == 0xEC) { // ì
        if (!top && args == -1)
            implicit_input(1,TYPE_INT);

        if (stack[top].type == TYPE_INT && args == -1) {
            itoa(stack[top].val,stack[top].val_str,10);
            stack[top].type = TYPE_STR;
        }
        else if (stack[top].type == TYPE_STR && args == -1) {
            stack[top].val = atoi(stack[top].val_str);
            stack[top].type = TYPE_INT;
        }
        else if (args != -1) {
            itoa(args,stack[top+1].val_str,10);
            stack[top+1].type = TYPE_STR;
            top++;
        }
    }
    else if (command[0] == '`') {
        if (args == -1)
            printf("%s",stack[top].type == TYPE_INT ? "int" : stack[top].type == TYPE_STR ? "string" : "float");
        else {
            if (args == 0)
                args = 1;
            else
                args = top - args;

            while (args < top) {
                if (stack[args].type == TYPE_INT)
                    printf("%d",stack[args].val);
                else if (stack[args].type == TYPE_STR)
                    printf("%s",stack[args].val_str);
                else if (stack[args].type == TYPE_FLT)
                    printf("%f",stack[args].val_flt);

                args++;
                if (args <= top)
                    putchar(' ');
            }
        }
    }
    else if (command[0] == 0xDF) { // ß (s)
        if (args == -1)
            args = 0;

        putchar(" \n"[args]);
    }
    /* THESE ARE TEMPORARY */
    else if (command[0] == 0x1F) { // ñ (NOT)
        if (!top && !args)
            implicit_input(1,TYPE_INT);

        int x = !stack[top].val;
        stack[top].val = x;
    }
    else if (command[0] == 0xB7) { // · XOR
        if (!top && args)
            implicit_input(2,TYPE_INT);

        int x = stack[top-1].val ^ stack[top].val;
        stack[top-1].val = x;
        zero(stack[top]);
        top--;
    }

    if (top >= cur_stack_size) {
        cur_stack_size += 1000;
        stack = realloc(stack,cur_stack_size * sizeof(struct _stack));
    }

    if (flags.db)
        debug(command,orig_args);

    return 1;
}

int implicit_input(int count, int type)
{
    while (count--) {
        if (type == TYPE_INT)
            scanf("%d",&stack[++top].val);
        else if (type == TYPE_FLT)
            scanf("%f",&stack[++top].val_flt);
        else if (type == TYPE_STR)
            fgets(stack[++top].val_str,999,stdin);

        if (stack[top].val_str[strlen(stack[top].val_str)-1] == '\n')
            stack[top].val_str[strlen(stack[top].val_str)-1] = 0;
        stack[top].type = type;
    }
}

void do_skip(void)
{
    fseek(in,-1,SEEK_CUR);
    wint_t c = getwc(in);

    if (c == L'{') {
        int braces = 1;
        while (braces) {
            c = getwc(in);
            if (c == L'{')
                braces++;
            else if (c == L'}')
                braces--;
        }
    }
    else {
        while (isdigit(wctob(c = getwc(in))));
        ungetwc(c,in);
    }
}

#define clear_jump(off) memset(&jumps[jumpnum+off],0,sizeof(struct _jump))

int do_jump(void)
{
    int j1, j2;

    if (jumps[jumpnum-1].val == -1)
        j1 = 1;
    else {
        struct _stack j1s;
        memcpy(&j1s,&stack[jumps[jumpnum-1].val],sizeof(struct _stack));
        j1 = j1s.type == TYPE_STR ? strlen(j1s.val_str) : j1s.type == TYPE_FLT ? j1s.val_flt : j1s.type == TYPE_INT ? j1s.val : 0;
    }

    if (jumps[jumpnum].val == -1)
        j2 = 1;
    else {
        struct _stack j2s;
        memcpy(&j2s,&stack[jumps[jumpnum].val],sizeof(struct _stack));
        j2 = j2s.type == TYPE_STR ? strlen(j2s.val_str) : j2s.type == TYPE_FLT ? j2s.val_flt : j2s.type == TYPE_INT ? j2s.val : 0;
    }

    //printf("do_jump with %d and %d\n",j1,j2);

    if (j1 && j2) {
        fseek(in, jumps[jumpnum-1].pos, SEEK_SET);
        jumped = 1;
        clear_jump(0);
        return 1;
    }
    else {
        clear_jump(-1);
        clear_jump(0);
        jumpnum--;
        return 0;
    }
}

void debug(wint_t *command, int args)
{
    printf("%S",command);
    if (args != -1)
        printf("%d",args);

    putchar('\t');
    int i;
    for (i = 1; i <= top; i++) {
        if (stack[i].type == TYPE_INT)
            printf("%d",stack[i].val);
        else if (stack[i].type == TYPE_STR)
            printf("%s",stack[i].val_str);
        else if (stack[i].type == TYPE_FLT)
            printf("%f",stack[i].val_flt);

        if (i != top)
            printf(", ");
    }
    putchar('\n');
    sleep(1);
}

/* wide-char string to int */

int wcstoi(wint_t *str)
{
    int retval = 0;

    while (*str)
    {
        retval *= 10;
        retval += *str - L'0';
        str++;
    }

    return retval;
}

#ifndef _WIN32
/* A utility function to reverse a string, credit to http://www.geeksforgeeks.org/implement-itoa/  */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        int tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
}
 
// Implementation of itoa()
char* itoa(int num, char* str, int base)
{
    int i = 0;
    int isNegative = 0;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = 1;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}
#endif

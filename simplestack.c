#include "simplestack.h"

struct _stack *stack, mem;
struct _jump  jumps[1000];
struct _flags flags;
int top, cur_stack_size;
int jumpnum, jumped;
int noprint, skip;
FILE *in;

/* canvas mode */
char *outbuf;
int cur_output_size;

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
          case 'i': flags.in = 1;       break;
          case 'd': flags.db = 1;       break;
          case 'q': flags.nn = 1;       break; // quiet
          case 'C': flags.cn = 1;       break;
        }
    }

    in = fopen(argv[2],"r");

    if (!in) {
        fprintf(stderr,"Error %d: Cannot open file %s: %s\n",errno,argv[2],strerror(errno));
        return 2;
    }

    struct stat sb;
    stat(argv[2], &sb);
    if (sb.st_size == 0)
        return coolstuff();

    outbuf = malloc(1000*sizeof(char));
    memset(outbuf,0,1000);
    cur_output_size = 1000;

    stack = malloc(1000*sizeof(struct _stack));
    memset(stack,0,1000*sizeof(struct _stack));
    cur_stack_size = 1000;

    int loop = 1, args = 0;
    wint_t buf[INPUT_BUFFER_SIZE] = {(wint_t)0};
    wint_t command[INPUT_BUFFER_SIZE] = {(wint_t)0};

    wcscpy(command,L"UNINITIALIZED");

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

    if (top && !noprint && !flags.cn) {
        if (stack[top].type == TYPE_INT)
            ssprintf("%d",stack[top].val);
        else if (stack[top].type == TYPE_STR)
            ssprintf("%s",stack[top].val_str);
        else if (stack[top].type == TYPE_FLT)
            ssprintf("%f",stack[top].val_flt);
    }

    if (flags.cn) {
        printf("%s",outbuf);
    }

    fflush(stdout);
    fclose(in);
    free(stack);
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
        else if (c == L'`') {
            c = getwc(in);
            char tmp[10] = {0};
            itoa(c,tmp,10);
            mbstowcs(buf,tmp,10);
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

void do_skip(int args)
{
    if (args == -1)
        fseek(in,-1,SEEK_CUR);
    wint_t c = getwc(in);

    if (c == L'{') {
        int braces = 1;
        while (braces && (c != WEOF)) {
            c = getwc(in);
            if (c == L'{')
                braces++;
            else if (c == L'}')
                braces--;
        }
    }
    else {
        ungetwc(c,in);
        skip = 1;
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
    if (!command[0])
        return;

    printf("%S",command);
    if (args != -1)
        printf("%d",args);

    putchar('\t');
    printf("0x%X",command[0]);
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

    if (flags.in)
        getchar();
    else
        sleep(1);
}

void stack_realloc(void)
{
    if (top >= cur_stack_size) {
        struct _stack s;
        memcpy(&s,&stack[top],sizeof(struct _stack));

        cur_stack_size += 1000;
        stack = realloc(stack,cur_stack_size * sizeof(struct _stack));

        memcpy(&stack[top],&s,sizeof(struct _stack));
    }
}

int append_to_outbuf(const char *s, ...)
{
    if (strlen(outbuf) > (cur_output_size - 100)) {
        cur_output_size += 1000;
        outbuf = realloc(outbuf,cur_output_size * sizeof(char));
    }

    va_list args;
    va_start(args,s);

    char buf[1000] = {0};
    int retval = vsprintf(buf,s,args);
    strcat(outbuf,buf);

    va_end(args);
    return retval;
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

// what to do upon empty program   https://codegolf.stackexchange.com/a/11423/61563
int coolstuff(void)
{
    int c;float a,b;scanf("%f",&a);while(scanf("%s%f",&c,&b)!=-1)c=='+'?a+=b:c=='-'?(a-=b):c=='*'?(a*=b):(a/=b);printf("%f",a);
}

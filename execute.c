#include "simplestack.h"

/* return value:

   1  continue loop unconditionally
   0  exit loop if EOF reached
   -1 exit loop unconditionally
*/

int execute(wint_t *command, int args)
{
    int orig_args = args; // for debugging, some commands modify args

    if (skip) {
        skip--;
        return 1;
    }

    if (!wcscmp(command,L"EOF")) {
        if (jumpnum) {
            jumps[jumpnum].val = -1;
            return do_jump();
        }
        return 0;
    }
    else if (!wcscmp(command,L"UNINITIALIZED")) {
        if (args != -1)
            stack[++top].val = args;
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
        if (top == 0 && args == -1)
            implicit_input(2,TYPE_INT);
        else if (top == 1 || (top == 0 && args != -1))
            implicit_input(1,TYPE_INT);

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
                ssprintf("%d",stack[top].val);
            else if (stack[top].type == TYPE_STR)
                ssprintf("%s",stack[top].val_str);
            else if (stack[top].type == TYPE_FLT)
                ssprintf("%f",stack[top].val_flt);
        }
        else
            ssprintf("%d",args);
    }
    else if (command[0] == L'<') {
        if (!top)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);
        else if (top == 1 && args == -1)
            implicit_input(1,TYPE_INT);

        if (args == -1)
        {
            float f1 = (stack[top-1].type == TYPE_FLT) ? stack[top-1].val_flt : (stack[top-1].type == TYPE_STR) ? strlen(stack[top-1].val_str) : (float) stack[top-1].val;
            float f2 = (stack[top].type == TYPE_FLT)   ? stack[top].val_flt   : (stack[top].type == TYPE_STR)   ? strlen(stack[top].val_str)   : (float) stack[top].val;

            stack[++top].val = (f1 < f2);
        }
        else {
            if (stack[top].type == TYPE_INT)
                stack[++top].val = (stack[top].val < args);
            else if (stack[top].type == TYPE_FLT)
                stack[++top].val = (stack[top].val_flt < args);
            else if (stack[top].type == TYPE_STR)
                stack[++top].val = (strlen(stack[top].val_str) < args);
        }
    }
    else if (command[0] == L'>') {
        if (top == 0)
            implicit_input(args == -1 ? 2 : 1,TYPE_INT);

        if (args == -1)
        {
            float f1 = (stack[top-1].type == TYPE_FLT) ? stack[top-1].val_flt : (stack[top-1].type == TYPE_STR) ? strlen(stack[top-1].val_str) : (float) stack[top-1].val;
            float f2 = (stack[top].type == TYPE_FLT)   ? stack[top].val_flt   : (stack[top].type == TYPE_STR)   ? strlen(stack[top].val_str)   : (float) stack[top].val;

            stack[++top].val = (f1 > f2);
        }
        else {
            if (stack[top].type == TYPE_INT)
                stack[++top].val = (stack[top].val > args);
            else if (stack[top].type == TYPE_FLT)
                stack[++top].val = (stack[top].val_flt > args);
            else if (stack[top].type == TYPE_STR)
                stack[++top].val = (strlen(stack[top].val_str) > args);
        }
        top++;
    }
    else if (command[0] == L'@') {
        if (args == -1)
            ssprintf("%c",stack[top].val);
        else
            ssprintf("%c",args);
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
    else if (command[0] == 0x00F7) {   // ÷ read float
        args = abs(args);
        while (args--) {
            scanf("%f",&stack[++top].val_flt);
            stack[top].type = TYPE_FLT;
        }
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
        args = (args == -1) ? 0 : args;

        while (top <= args)
            implicit_input(1,TYPE_INT);

        memcpy(&mem,&stack[top-args],sizeof(struct _stack));
        zero(stack[top]);
        top--;
    }
    else if (command[0] == L']') {
        args = abs(args);

        while (args--)
            memcpy(&stack[++top],&mem,sizeof(struct _stack));
    }
    else if (command[0] == 0xA4) {
        //builtin(args);
    }
    else if (command[0] == L'?') {
        int val, x = stack[top].type == TYPE_INT ? stack[top].val : stack[top].type == TYPE_FLT ? stack[top].val_flt : strlen(stack[top].val_str);
        if (args == -1)
            val = x;
        else
            val = (args == x);


        if (!val)
            do_skip(args);
    }
    else if (command[0] == L'!') {
        int val, x = stack[top].type == TYPE_INT ? stack[top].val : stack[top].type == TYPE_FLT ? stack[top].val_flt : strlen(stack[top].val_str);
        if (args == -1)
            val = x;
        else
            val = (args == x);


        if (val)
            do_skip(args);
    }
    else if (command[0] == L'(') {
        if (args == -1) {
            jumps[jumpnum].val = -1;
            jumps[jumpnum].pos = ftell(in) - 1;
            jumpnum++;
        }
        else {
            int j = stack[top-args].val;
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
            if (args != -1) {
                if (stack[top].val == args)
                    return -1;
            } else {
                if (stack[top].val)
                    return -1;
            }
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
            if (args != -1) {
                if (stack[top].val == args) {
                    noprint = 1;
                    return -1;
                }
            } else {
                if (stack[top].val) {
                    noprint = 1;
                    return -1;
                }
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
            ssprintf("%s",stack[top].type == TYPE_INT ? "int" : stack[top].type == TYPE_STR ? "string" : "float");
        else {
            if (args == 0)
                args = 1;
            else
                args = top - args;

            while (args < top) {
                if (stack[args].type == TYPE_INT)
                    ssprintf("%d",stack[args].val);
                else if (stack[args].type == TYPE_STR)
                    ssprintf("%s",stack[args].val_str);
                else if (stack[args].type == TYPE_FLT)
                    ssprintf("%f",stack[args].val_flt);

                args++;
                if (args <= top)
                    ssputchar(' ');
            }
        }
    }
    else if (command[0] == 0xDF) { // ß (s)
        if (args == -1)
            args = 0;

        ssputchar(" \n"[args]);
    }
    /* THESE ARE TEMPORARY */
    else if (command[0] == 0xF1) { // ñ (NOT)
        if (!top && !args)
            implicit_input(1,TYPE_INT);

        if (stack[top].type == TYPE_INT && args == -1) {
            int x = !stack[top].val;
            stack[top].val = x;
        }
        else if (stack[top].type == TYPE_FLT && args == -1) {
            int x = !stack[top].val_flt;
            stack[top].val = x;
            stack[top].type = TYPE_INT;
        }
    }
    else if (command[0] == 0xB7) { // · XOR
        if (!top && args)
            implicit_input(2,TYPE_INT);

        if (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT && args == -1) {
            int x = stack[top-1].val ^ stack[top].val;
            stack[top-1].val = x;
            zero(stack[top]);
            top--;
        }
    }
    else if (command[0] == 0xA9) { // © consume input
        if (args == -1) {
            int c;
            while ((c = getchar()) != EOF) {
                stack[++top].val = c;
                stack_realloc();
            }
        }
        else if (args == 1) {
            int i;
            while (scanf("%d",&i) != -1) {
                stack[++top].val = i;
                stack_realloc();
            }
        }
        else if (args == 2) {
            char s[STACK_STRING_SIZE] = {0};
            while (fgets(s,999,stdin)) {
                stack[++top].type = TYPE_STR;
                strcpy(stack[top].val_str,s);
                stack_realloc();
            }
        }
    }
    else if (command[0] == 0xAF) { // ¯ equivalent to []
        args = (args == -1) ? 0 : args;

        while (top <= args)
            implicit_input(1,TYPE_INT);

        memcpy(&mem,&stack[top-args],sizeof(struct _stack));
    }
    else if (command[0] == 0xAE) { // ® reverse stack
        int i, j = top;
        struct _stack *dest = malloc(cur_stack_size * sizeof(struct _stack));
        memset(dest,0,cur_stack_size * sizeof(struct _stack));

        for (i = 0; i < j; i++)
            memcpy(&dest[i+1],&stack[j-i],sizeof(struct _stack));

        memcpy(stack,dest,cur_stack_size * sizeof(struct _stack));

        free(dest);
    }
    else if (command[0] == 0xBD) { // ½ log2
        if (args == -1 && !top)
            implicit_input(1,TYPE_FLT);

        if (stack[top].type == TYPE_INT && args == -1) {
            float x = log2(stack[top].val);

            if (!stack[top].val)
                x = 0;

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
        else if (stack[top].type == TYPE_FLT && args == -1) {
            float x = log2(stack[top].val_flt);

            if (!stack[top].val_flt)
                x = 0;

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
        else if (args) {
            float x = log2(args);

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
    }
    else if (command[0] == 0xC1) { // Á log10
        if (args == -1 && !top)
            implicit_input(1,TYPE_FLT);

        if (stack[top].type == TYPE_INT && args == -1) {
            float x = log10(stack[top].val);

            if (!stack[top].val)
                x = 0;

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
        else if (stack[top].type == TYPE_FLT && args == -1) {
            float x = log10(stack[top].val_flt);

            if (!stack[top].val_flt)
                x = 0;

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
        else if (args) {
            float x = log10(args);

            stack[top].val_flt = x;
            stack[top].type = TYPE_FLT;
        }
    }
    else if (command[0] == 0xF6) { // ö is whole
        if (!top && args == -1)
            implicit_input(1,TYPE_FLT);

        if (stack[top].type == TYPE_FLT && args == -1) {
            int x = (floor(stack[top].val_flt) == ceil(stack[top].val_flt)) && (stack[top].val_flt >= 0);

            stack[++top].val = x;
        }
        else if (stack[top].type == TYPE_INT && args == -1) {
            if (args != -1) {
                if (stack[top].val != args) {
                    noprint = 1;
                    return -1;
                }
            } else {
                if (!stack[top].val) {
                    noprint = 1;
                    return -1;
                }
            }
        }
    }
    else if (command[0] == 0xD7) { // × split into chunks
        if (!top && args == -1) {
            implicit_input(1,TYPE_STR);
            implicit_input(1,TYPE_INT);
        }
        else if (top == 1 && args)
            implicit_input(1,TYPE_STR);

        if (stack[top-1].type == TYPE_STR && stack[top].type == TYPE_INT && args == -1) {
            args = stack[top].val;
            zero(stack[top]);
            top--;
        }

        if (stack[top].type == TYPE_STR && args > 0) {
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
    else if (command[0] == 0xB5) { // µ print entire stack
        if (!top)
        {
            if (args == -1) {
                int c;
                while ((c = getchar()) != EOF) {
                    stack[++top].val = c;
                    stack_realloc();
                }
            }
            else if (args == 1) {
                int i;
                while (scanf("%d",&i) != -1) {
                    stack[++top].val = i;
                    stack_realloc();
                }
            }
            else if (args == 2) {
                char s[STACK_STRING_SIZE] = {0};
                while (fgets(s,999,stdin)) {
                    stack[++top].type = TYPE_STR;
                    strcpy(stack[top].val_str,s);
                    stack[top].type = TYPE_STR;
                    stack_realloc();
                }
            }
        }

        int i;
        for (i = 1; i <= top; i++) {
            if (stack[i].type == TYPE_INT) {
                if (args == -1)
                    ssputchar(stack[i].val);
                else
                    ssprintf("%d",stack[i].val);
            }
            else if (stack[i].type == TYPE_STR)
                ssprintf("%s",stack[i].val_str);
            else if (stack[i].type == TYPE_FLT)
                ssprintf("%f",stack[i].val_flt);
        }
    }
    else if (command[0] == 0xF2) { // ò decrement memory
        args = abs(args);
        mem.val -= args;
    }
    else if (command[0] == 0xF3) { // ó increment memory
        args = abs(args);
        mem.val += args;
    }
    else if (command[0] == 0xEF) { // ï execute next command/block if top of stack equal to args, if implicit, if top of stack truthy
        if (stack[top].val != args) {
            printf("stack = %d  args = %d\n",stack[top].val,args);
            do_skip(args);
        }
    }
    else if (command[0] == 0xF5) {
        if (args != -1) {
            if (stack[top].val != args)
                noprint = 1;
        } else {
            if (!stack[top].val)
                noprint = 1;
        }
    }
    else if (command[0] == 0xFE) { // þ reverse canvas
        if (flags.cn) {
            int i, j = strlen(outbuf);
            char *dest = strdup(outbuf);

            for (i = 0; i < j; i++)
                dest[i] = outbuf[j-i-1];  // -1 because j is 1-indexed
            dest[i] = 0; // dafuq?

            strcpy(outbuf,dest);
            free(dest);
        }
    }
    else if (command[0] == 0xFF) { // ÿ push as binary
        if (!top && args == -1)
            implicit_input(1,TYPE_INT);

        if (stack[top].type == TYPE_INT && args == -1) {
            int x = stack[top].val;
            char *s = malloc(42*sizeof(char));
            itoa(x,s,2);
            stack[++top].val = atoi(s);
            free(s);
        }
    }
    else if (command[0] == 0xD8) { // Ø push strcmp
        if (!top && args == -1)
            implicit_input(2,TYPE_STR);
        else if (top == 1 && args == -1)
            implicit_input(1,TYPE_STR);

        if (stack[top-1].type == TYPE_STR && stack[top].type == TYPE_STR) {
            stack[top+1].val = strcmp(stack[top-1].val_str,stack[top].val_str);
            top++;
        }
    }

    stack_realloc();

    if (flags.db)
        debug(command,orig_args);

    return 1;
}
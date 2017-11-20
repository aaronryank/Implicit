#include "simplestack.h"

int exec_command_add(int args)
{
    if (top == 0)
	implicit_input(args == -1 ? 2 : 1,TYPE_INT);

    if (top > 1 && args == -1 && ((stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT)))
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

    return 1;
}

int exec_command_increment(int args)
{
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

    return 1;
}

int exec_command_sub(int args)
{
    if (top == 0)
	implicit_input(args == -1 ? 2 : 1,TYPE_INT);

    if (top > 1 && args == -1 && ((stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT)))
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

    return 1;
}

int exec_command_mul(int args)
{
    if (top == 0)
	implicit_input(args == -1 ? 2 : 1,TYPE_INT);

    if (top > 1 && args == -1 && ((stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT)))
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

    return 1;
}

int exec_command_div(int args)
{
    if (top == 0)
	implicit_input(args == -1 ? 2 : 1,TYPE_INT);

    if (top > 1 && args == -1 && ((stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT) || (stack[top-1].type == TYPE_FLT && stack[top].type == TYPE_FLT)))
    {
	if (args == -1 && top > 1) {
	    stack[top-1].val /= stack[top].val;
	    stack[top-1].val_flt /= stack[top].val_flt;
	    zero(stack[top]);
	    top--;
	}
	else {
	    if (abs(args)) {
		stack[top].val /= abs(args);
		stack[top].val_flt /= abs(args);
	    }
	}
    }
    else if ((stack[top].type == TYPE_INT || stack[top].type == TYPE_FLT) && args != -1)
    {
	if (args) {
	    stack[top].val /= args;
	    stack[top].val_flt /= args;
	}
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

    return 1;
}

int exec_command_mod(int args)
{
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

    return 1;
}

int exec_command_print(int args)
{
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

    return 1;
}

int exec_command_lt(int args)
{
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

    return 1;
}

int exec_command_gt(int args)
{
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

    return 1;
}

int exec_command_putchar(int args)
{
    if (args == -1)
	ssprintf("%c",stack[top].val);
    else
	ssprintf("%c",args);

    return 1;
}

int exec_command_exit(int args)
{
    if (args) {
	noprint = 1;
	return -1;
    }

    return 1;
}

int exec_command_getchar(int args)
{
    args = abs(args);
    while (args--)
	stack[++top].val = getchar();

    return 1;
}

int exec_command_getint(int args)
{
    args = abs(args);
    while (args--)
	scanf("%d",&stack[++top].val);  // int type by default

    return 1;
}

int exec_command_getstr(int args)
{
    args = abs(args);
    while (args--)
    {
	fgets(stack[++top].val_str,999,stdin);
	if (stack[top].val_str[strlen(stack[top].val_str)-1] == '\n')
	    stack[top].val_str[strlen(stack[top].val_str)-1] = 0;
	stack[top].type = TYPE_STR;
    }

    return 1;
}

int exec_command_duplicate(int args)
{
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

    return 1;
}

int exec_command_pop(int args)
{
    args = abs(args);
    while (args-- && top) {
	zero(stack[top]);
	top--;
    }

    return 1;
}

int exec_command_reverse(int args)
{
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
	    char *s = strdup(stack[top].val_str);
	    memset(stack[top].val_str,0,STACK_STRING_SIZE);
	    strcpy(stack[top].val_str,&s[args]);
	    free(s);
	}
    }

    return 1;
}

int exec_command_raise(int args)
{
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

    return 1;
}

int exec_command_stringify(int args)
{
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

    return 1;
}

int exec_command_stack_len(int args)
{
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

    return 1;
}

int exec_command_eq(int args)
{
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

    return 1;
}

int exec_command_swap(int args)
{
    if (args == -1)
	args = top-1;
    else
	args++;  // to avoid swapping with 0th stack index

    struct _stack tmp;
    memcpy(&tmp,&stack[args],sizeof(struct _stack));
    memcpy(&stack[args],&stack[top],sizeof(struct _stack));

    memcpy(&stack[top],&tmp,sizeof(struct _stack));

    return 1;
}

int exec_command_tomem(int args)
{
    args = (args == -1) ? 0 : args;

    while (top <= args)
	implicit_input(1,TYPE_INT);

    memcpy(&mem,&stack[top-args],sizeof(struct _stack));
    zero(stack[top]);
    top--;

    return 1;
}

int exec_command_frommem(int args)
{
    args = abs(args);

    while (args--)
	memcpy(&stack[++top],&mem,sizeof(struct _stack));

    return 1;
}

int get_top_val(int args)
{
    int val, x = stack[top].type == TYPE_INT ? stack[top].val : stack[top].type == TYPE_FLT ? stack[top].val_flt : strlen(stack[top].val_str);

    if (args == -1)
	val = x;
    else
	val = (args == x);

    return val;
}

int exec_command_if(int args)
{
    if (!get_top_val(args))
	do_skip(args);

    return 1;
}

int exec_command_ifnot(int args)
{
    if (get_top_val(args))
	do_skip(args);

    return 1;
}

int exec_command_do(int args)
{
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

    return 1;
}

int exec_command_while(int args)
{
    if (args == -1) {
	jumps[jumpnum].val = top;
	return do_jump();
    }
    else {
	jumps[jumpnum].val = args+1;
	return do_jump();
    }

    return 1;
}

int exec_command_dupstack(int args)
{
    int i;
    for (i = 1; i <= top; i++)
	memcpy(&stack[i+top],&stack[i],sizeof(struct _stack));
    top += top;

    return 1;
}

int exec_command_getflt(int args)
{
    args = abs(args);
    while (args--) {
	scanf("%f",&stack[++top].val_flt);
	stack[top].type = TYPE_FLT;
    }

    return 1;
}

int exec_command_str(int args, wint_t *command)
{
    args = abs(args);
    while (args--)
    {
	wcstombs(stack[++top].val_str,&command[1],INPUT_BUFFER_SIZE);
	stack[top].type = TYPE_STR;
    }

    return 1;
}

int exec_command_swap_case(int args)
{
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

    return 1;
}

int exec_command_loop_forever(int args)
{
    jumps[jumpnum].val = -1;
    return do_jump();

    return 1;
}

int exec_command_alpha(int args)
{
    args = abs(args);

    while (args--) {
	strcpy(stack[++top].val_str,"abcdefghijklmnopqrstuvwxyz");
	stack[top].type = TYPE_STR;
    }

    return 1;
}

int exec_command_decrement(int args)
{
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

    return 1;
}

int exec_command_incstr(int args)
{
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

    return 1;
}

int exec_command_decstr(int args)
{
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

    return 1;
}

int exec_command_strchar(int args)
{
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

    return 1;
}

int exec_command_sign(int args)
{
    if (!top)
	implicit_input(1,TYPE_INT);

    if (args == -1 && stack[top].type == TYPE_INT) {
	stack[top+1].val = stack[top].val < 0 ? -1 : stack[top].val == 0 ? 0 : 1;
	top++;
    }
    else if (args != -1) {
	stack[++top].val = -args;
    }

    return 1;
}

int exec_command_seq(int args)
{
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

    return 1;
}

int exec_command_sum_stack(int args)
{
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

    return 1;
}

int exec_command_itoa(int args)
{
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

    return 1;
}

int exec_command_whitespace(int args)
{
    if (args == -1)
	args = 0;

    ssputchar(" \n"[args]);

    return 1;
}


int exec_command_consume(int args)
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
	    stack_realloc();
	}
    }

    return 1;
}

int exec_command_log10(int args)
{
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

    return 1;
}

int exec_command_xor(int args)
{
    if (!top && args)
	implicit_input(2,TYPE_INT);

    if (stack[top-1].type == TYPE_INT && stack[top].type == TYPE_INT && args == -1) {
	int x = stack[top-1].val ^ stack[top].val;
	stack[top-1].val = x;
	zero(stack[top]);
	top--;
    }

    return 1;
}

int exec_command_copy_to_mem(int args)
{
    args = (args == -1) ? 0 : args;

    while (top <= args)
	implicit_input(1,TYPE_INT);

    memcpy(&mem,&stack[top-args],sizeof(struct _stack));

    return 1;
}

int exec_command_not(int args)
{
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

    return 1;
}

int exec_command_rev_stack(int args)
{
    int i, j = top;
    struct _stack *dest = malloc(cur_stack_size * sizeof(struct _stack));
    memset(dest,0,cur_stack_size * sizeof(struct _stack));

    for (i = 0; i < j; i++)
	memcpy(&dest[i+1],&stack[j-i],sizeof(struct _stack));

    memcpy(stack,dest,cur_stack_size * sizeof(struct _stack));

    free(dest);

    return 1;
}

int exec_command_log2(int args)
{
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

    return 1;
}

int exec_command_iswhole(int args)
{
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

    return 1;
}

int exec_command_split(int args)
{
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

    return 1;
}

int exec_command_print_stack(int args)
{
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

    return 1;
}

int exec_command_strcmp(int args)
{
    if (!top && args == -1)
	implicit_input(2,TYPE_STR);
    else if (top == 1 && args == -1)
	implicit_input(1,TYPE_STR);

    if (stack[top-1].type == TYPE_STR && stack[top].type == TYPE_STR) {
	stack[top+1].val = strcmp(stack[top-1].val_str,stack[top].val_str);
	top++;
    }

    return 1;
}

int exec_command_push_bin(int args)
{
    if (!top && args == -1)
	implicit_input(1,TYPE_INT);

    if (stack[top].type == TYPE_INT && args == -1) {
	int x = stack[top].val;
	char *s = malloc(42*sizeof(char));
	itoa(x,s,2);
	stack[++top].val = atoi(s);
	free(s);
    }

    return 1;
}

int exec_command_rev_canvas(int args)
{
    if (flags.cn) {
	int i, j = strlen(outbuf);
	char *dest = strdup(outbuf);

	for (i = 0; i < j; i++)
	    dest[i] = outbuf[j-i-1];  // -1 because j is 1-indexed
	dest[i] = 0; // dafuq?

	strcpy(outbuf,dest);
	free(dest);
    }

    return 1;
}

int exec_command_if_eq(int args)
{
    if (stack[top].val != args) {
	printf("stack = %d  args = %d\n",stack[top].val,args);
	do_skip(args);
    }

    return 1;
}

int exec_command_exit_thingy(int args)
{
    if (args != -1) {
	if (stack[top].val != args)
	    noprint = 1;
    } else {
	if (!stack[top].val)
	    noprint = 1;
    }

    return 1;
}

int exec_command_inc_mem(int args)
{
    args = abs(args);
    mem.val += args;

    return 1;
}

int exec_command_dec_mem(int args)
{
    args = abs(args);
    mem.val -= args;

    return 1;
}

int execute(wint_t *command, int args)
{
    int orig_args = args; // for debugging, some commands modify args
    int ret = 1;

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
    else if (isalpha(wctob(command[0]))) {
	args = abs(args);
	while (args--)
	    stack[++top].val = command[0];
    }
    else {
	switch (*command) {
	    case L'-': ret = exec_command_sub(args);         break;
	    case L'!': ret = exec_command_ifnot(args);       break;
	    case L'#': ret = exec_command_stack_len(args);   break;
	    case L'$': ret = exec_command_getint(args);      break;
	    case L'%': ret = exec_command_print(args);       break;
	    case L'&': ret = exec_command_exit(args);        break;
	    case L'(': ret = exec_command_do(args);          break;
	    case L')': ret = exec_command_while(args);       break;
	    case L'*': ret = exec_command_mul(args);         break;
	    case L',': ret = exec_command_swap(args);        break;
	    case L'.': ret = exec_command_increment(args);   break;
	    case L'/': ret = exec_command_div(args);         break;
	    case L':': ret = exec_command_duplicate(args);   break;
	    case L';': ret = exec_command_pop(args);         break;
	    case L'?': ret = exec_command_if(args);          break;
	    case L'@': ret = exec_command_putchar(args);     break;
	    case L'[': ret = exec_command_tomem(args);       break;
	    case L'\'':ret = exec_command_getstr(args);      break;
	    case L'\"':ret = exec_command_stringify(args);   break;
	    case L'\\':ret = exec_command_reverse(args);     break;
	    case L']': ret = exec_command_frommem(args);     break;
	    case L'^': ret = exec_command_raise(args);       break;
	    case L'_': ret = exec_command_mod(args);         break;
	    case L'|': ret = exec_command_dupstack(args);    break;
	    case L'~': ret = exec_command_getchar(args);     break;
	    case L'+': ret = exec_command_add(args);         break;
	    case L'<': ret = exec_command_lt(args);          break;
	    case L'=': ret = exec_command_eq(args);          break;
	    case L'>': ret = exec_command_gt(args);          break;

	    case 0xA1: ret = exec_command_seq(args);         break; // ¡
	    case 0xA6: ret = exec_command_swap_case(args);   break; // ¦
	    case 0xA7: ret = exec_command_strchar(args);     break; // §
	    case 0xA9: ret = exec_command_consume(args);     break; // ©
	    case 0xAB: ret = exec_command_str(args,command); break; // «
	    case 0xAE: ret = exec_command_rev_stack(args);   break; // ®
	    case 0xAF: ret = exec_command_copy_to_mem(args); break; // ¯
	    case 0xB1: ret = exec_command_sign(args);        break; // ±
	    case 0xB5: ret = exec_command_print_stack(args); break; // µ
	    case 0xB6: ret = exec_command_loop_forever(args);break; // ¶
	    case 0xB7: ret = exec_command_xor(args);         break; // ·
	    case 0xB8: ret = exec_command_decrement(args);   break; // broken
	    case 0xBD: ret = exec_command_log2(args);        break; // ½
	    case 0xC0: ret = exec_command_alpha(args);       break; // À
	    case 0xC1: ret = exec_command_log10(args);       break; // Á
	    case 0xD7: ret = exec_command_split(args);       break; // ×
	    case 0xD8: ret = exec_command_strcmp(args);      break; // Ø
	    case 0xDE: ret = exec_command_sum_stack(args);   break; // 
	    case 0xDF: ret = exec_command_whitespace(args);  break; // ß
	    case 0xE8: ret = exec_command_decstr(args);      break; // è
	    case 0xE9: ret = exec_command_incstr(args);      break; // é
	    case 0xEC: ret = exec_command_itoa(args);        break; // ì
	    case 0xEF: ret = exec_command_if_eq(args);       break; // ï
	    case 0xF1: ret = exec_command_not(args);         break; // ñ
	    case 0xF2: ret = exec_command_dec_mem(args);     break; // ò
	    case 0xF3: ret = exec_command_inc_mem(args);     break; // ó
	    case 0xF5: ret = exec_command_exit_thingy(args); break; // idk
	    case 0xF6: ret = exec_command_iswhole(args);     break; // ö
	    case 0xF7: ret = exec_command_getflt(args);      break; // ÷
	    case 0xFE: ret = exec_command_rev_canvas(args);  break; // þ
	    case 0xFF: ret = exec_command_push_bin(args);    break; // ÿ
	}
    }

    stack_realloc();

    if (flags.db)
	debug(command,orig_args);

    return ret;
}
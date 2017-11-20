#include "simplestack.h"
#include "commands.h"

int exec_command_add(int args)
{
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);

    if (top > 1 && implicit && compatibletypes(-1,0))
    {
        float f1 = (float) typeval(0);

	val_int(-1) += (int) f1;
	val_flt(0)  += f1;

	zero(stack[top]);
	top--;
    }
    else if (type_str(-1))
    {
	if (!type_str(0))
	    val_str(0)[0] = val_int(0);

        args = abs(args);

	while (args--)
	    strcat(val_str(-1),val_str(0));

	zero(stack[top]);
	top--;
    }
    else if (type_num(0))
    {
        args = abs(args);

	val_int(0) += args;
	val_flt(0) += args;
    }

    return 1;
}

int exec_command_increment(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_INT);
    else if (!top && !implicit)
	implicit_input(1,TYPE_STR);

    if (type_int(0) && implicit)
	val_int(0)++;
    else if (type_str(0) && implicit) {                    /* I forgot what this does ._. */
	int i, l = strlen(val_str(0));

	if (args >= l)
	    return 1;

        args = abs(args);

	char s[STACK_STRING_SIZE] = {0};
	strcpy(s,val_str(0));
	memset(val_str(0),0,STACK_STRING_SIZE);

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
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);

    if (top > 1 && implicit && compatibletypes(-1,0))
    {
        float f1 = (float) typeval(0);

	val_int(-1) -= (int) f1;
	val_flt(-1) -= f1;

	zero(stack[top]);
	top--;
    }
    else if (args && type_num(0))
    {
        args = abs(args);

	val_int(0) -= args;
	val_flt(0) -= (float) args;
    }
    else if (type_str(0))
    {
        args = abs(args);
	while (args--)
	    val_str(0)[strlen(val_str(0))-1] = 0;
    }

    return 1;
}

int exec_command_mul(int args)
{
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);

    if (top > 1 && implicit && compatibletypes(-1,0))
    {
        float f1 = (float) typeval(0);

	val_int(-1) *= (int) f1;
	val_flt(-1) *= f1;

	zero(stack[top]);
	top--;
    }
    else if (type_str(-1))
    {
	if (implicit)
	    args = 0;

	if (type_int(0))
	    val_str(0)[0] = val_int(0);

	int sz = strlen(val_str(0));

	/* make space in the string for the characters */
	char *t1 = malloc(strlen(&val_str(-1)[args]) * sizeof(char));
	strcpy(t1,&val_str(-1)[args]);
	strcpy(&val_str(-1)[args+sz],t1);
	free(t1);

	/* add the characters */
	int i;
	for (i = 0; i < sz; i++)
	    val_str(-1)[args+i] = val_str(0)[i];

	zero(stack[top]);
	top--;
    }
    else if (type_num(0) && implicit)
    {
        args = abs(args);
	val_int(0) *= args;
	val_flt(0) *= args;
    }

    return 1;
}

int exec_command_div(int args)
{
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);

    if (top > 1 && implicit && compatibletypes(-1,0))
    {
        float f1 = (float) typeval(0);

	val_int(-1) /= (int) f1;
	val_flt(-1) /= f1;

	zero(stack[top]);
	top--;
    }
    else if (type_num(0) && !implicit)
    {
	if (args) {
	    val_int(0) /= args;
	    val_flt(0) /= args;
	}
    }
    else if (type_str(0))
    {
	if (implicit)
	    args = 0;

	if (type_int(0))
	    val_str(0)[0] = val_int(0);

	int sz = strlen(val_str(0));

	/* add the characters */
	int i;
	for (i = 0; i < sz; i++)
	    val_str(-1)[args+i] = val_str(0)[i];

	zero(stack[top]);
	top--;
    }

    return 1;
}

int exec_command_mod(int args)
{
    if (!top && implicit)
	implicit_input(2,TYPE_INT);
    else if (top == 1 || (!top && !implicit))
	implicit_input(1,TYPE_INT);

    if (top > 1 && type_int(-1) && type_int(0) && implicit)
    {
	val_int(-1) %= val_int(0);
	zero(stack[top]);
	top--;
    }
    if (top > 1 && type_flt(-1) && type_flt(0) && implicit)
    {
	val_int(-1) = fmod(val_flt(-1),val_flt(0));
	zero(stack[top]);
	top--;
    }
    else if (type_int(0) && !implicit)
    {
	val_int(0) %= args;
    }
    else if (type_str(0)) {
	if (implicit)
	    args = strlen(val_str(0))-1;

	stack[top+1].val = val_str(0)[args];
	top++;
    }

    return 1;
}

int exec_command_print(int args)
{
    if (implicit) {
	if (type_int(0))
	    ssprintf("%d",val_int(0));
	else if (type_str(0))
	    ssprintf("%s",val_str(0));
	else if (stack[top].type == TYPE_FLT)
	    ssprintf("%f",val_flt(0));
    }
    else
	ssprintf("%d",args);

    return 1;
}

int exec_command_lt(int args)
{
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);
    else if (top == 1 && implicit)
	implicit_input(1,TYPE_INT);

    if (implicit)
    {
	float f1 = (float) typeval(-1);
	float f2 = (float) typeval(0);

	val_int(1) = (int) f1 < f2;
	top++;
    }
    else {
        float f1 = (float) typeval(0);
	float f2 = (float) args;

        val_int(1) = (int) f < args;
        top++;
    }

    return 1;
}

int exec_command_gt(int args)
{
    if (!top)
	implicit_input(implicit ? 2 : 1,TYPE_INT);
    else if (top == 1 && implicit)
	implicit_input(1,TYPE_INT);

    if (implicit)
    {
	float f1 = (float) typeval(-1);
	float f2 = (float) typeval(0);

	val_int(1) = (int) f1 > f2;
	top++;
    }
    else {
        float f1 = (float) typeval(0);
	float f2 = (float) args;

        val_int(1) = (int) f > args;
        top++;
    }

    return 1;
}

int exec_command_putchar(int args)
{
    if (implicit)
	ssprintf("%c",val_int(0));
    else
	ssprintf("%c",args);

    return 1;
}

int exec_command_exit(int args)
{
    int ret = 0;

    if (implicit)
        ret = 1;
    else
        ret = typeval(-args);

    if (ret) {
	noprint = 1;
	return -1;
    }

    return 1;
}

int exec_command_getchar(int args)
{
    args = abs(args);

    while (args--) {
	val_int(1) = getchar();
        top++;
    }

    return 1;
}

int exec_command_getint(int args)
{
    args = abs(args);

    while (args--) {
	scanf("%d",&val_int(1));
        top++;
    }

    return 1;
}

int exec_command_getstr(int args)
{
    args = abs(args);
    while (args--)
    {
	fgets(val_str(1),999,stdin);
        top++;

	if (val_str(0)[strlen(val_str(0))-1] == '\n')
	    val_str(0)[strlen(val_str(0))-1] = 0;

	stack[top].type = TYPE_STR;
    }

    return 1;
}

int exec_command_duplicate(int args)
{
    if (implicit && !top)
	implicit_input(1,TYPE_INT);

    if (implicit) {
	val_int(1) = val_int(0);
	val_flt(1) = val_flt(0);
        type(1) = type(0);

        strcpy(val_str(1),val_str(0));
    }
    else
	val_int(1) = args; // int type by default

    top++;

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

    if (implicit) {
	if (type_int(0))
	    val_int(0) = -val_int(0);
	else if (stack[top].type == TYPE_FLT)
	    val_flt(0) = -val_flt(0);
	else {
	    int i, j = strlen(val_str(0));
	    char *dest = strdup(val_str(0));

	    for (i = 0; i < j; i++)
		dest[i] = val_str(0)[j-i-1];  // -1 because j is 1-indexed
	    dest[i] = 0; // dafuq?

	    strcpy(val_str(0),dest);
	    free(dest);
	}
    }
    else {
	if (type_str(0) && args) {
	    char *s = strdup(val_str(0));
	    memset(val_str(0),0,STACK_STRING_SIZE);
	    strcpy(val_str(0),&s[args]);
	    free(s);
	}
    }

    return 1;
}

int exec_command_raise(int args)
{
    if (!top)
	implicit_input(1,TYPE_INT);

    if (type_int(0)) {
	args = implicit ? 2 : args;
	while (--args)
	    val_int(0) *= val_int(0);
    }
    else if (stack[top].type == TYPE_FLT) {
	args = implicit ? 2 : args;
	while (--args)
	    val_flt(0) *= val_flt(0);
    }
    else if (type_str(0)) {
	if (implicit) {
	    stack[top+1].val = strlen(val_str(0));
	    top++;
	}
	else
	    memset(&val_str(0)[args],0,STACK_STRING_SIZE - args - 1);
    }

    return 1;
}

int exec_command_stringify(int args)
{
    int start;
    if (implicit)
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
	top--; args--;
    }
    memcpy(&stack[++top],&tmp,sizeof(struct _stack));

    return 1;
}

int exec_command_stack_len(int args)
{
    if (implicit) {
	val_int(1) = top;
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
    if (!top)
	implicit_input(2,TYPE_INT);

    if (implicit)
    {
	float f1 = typeval(-1);
	float f2 = typeval(0);

	if (type_str(-1))
	    val_int(1) = !strcmp(val_str(-1),val_str(0));
	else
	    val_int(1) = (f1 == f2);
    }
    else {
	val_int(1) = (val_int(0) == args);
    }

    top++;

    return 1;
}

int exec_command_swap(int args)
{
    if (implicit)
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
    args = implicit ? 0 : args;

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
    int val, x = type_int(0) ? val_int(0) : stack[top].type == TYPE_FLT ? val_flt(0) : strlen(val_str(0));

    if (implicit)
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
    if (implicit) {
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
    if (implicit) {
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

    if (type_int(0)) {
	if (implicit) {
	    if (isupper(val_int(0)))
		val_int(0) = tolower(val_int(0));
	    else
		val_int(0) = toupper(val_int(0));
	}
	else if (args == 1)
	    val_int(0) = tolower(val_int(0));
	else if (args == 2)
	    val_int(0) = toupper(val_int(0));
    }
    else if (type_str(0)) {
	int i;
	if (implicit)
	{
	    int l = strlen(val_str(0));
	    for (i = 0; i < l; i++) {
		if (isupper(val_str(0)[i]))
		    val_str(0)[i] = tolower(val_str(0)[i]);
		else if (islower(val_str(0)[i]))
		    val_str(0)[i] = toupper(val_str(0)[i]);
	    }
	}
	else if (args == 1)
	    for (i = 0; val_str(0)[i]; i++)
		val_str(0)[i] = tolower(val_str(0)[i]);
	else if (args == 2)
	    for (i = 0; val_str(0)[i]; i++)
		val_str(0)[i] = toupper(val_str(0)[i]);
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

    if (type_str(0) && implicit) {
	int i, l = strlen(val_str(0));
	for (i = 0; i < l; i++)
	    stack[top+i+1].val = val_str(0)[i];
	zero(stack[top]);
	top += l - 1;
    }
    else if (type_int(0) && implicit) {
	val_int(0)--;
    }

    return 1;
}

int exec_command_incstr(int args)
{
    if (!top)
	implicit_input(1,TYPE_STR);

    if (type_str(0)) {
	int i, l = strlen(val_str(0));
	for (i = 0; i < l; i++)
	    val_str(0)[i] += abs(args);
    }
    else if (type_int(0) && implicit) {
	if (!implicit) {
	    if (val_int(0) == args)
		return -1;
	} else {
	    if (val_int(0))
		return -1;
	}
    }

    return 1;
}

int exec_command_decstr(int args)
{
    if (!top)
	implicit_input(1,TYPE_STR);

    if (type_str(0) && args) {
	int i, l = strlen(val_str(0));
	for (i = 0; i < l; i++)
	    val_str(0)[i] -= abs(args);
    }
    else if (type_int(0) && implicit) {
	if (!implicit) {
	    if (val_int(0) == args) {
		noprint = 1;
		return -1;
	    }
	} else {
	    if (val_int(0)) {
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

    if (type_str(0)) {
	char *s = strdup(val_str(0));
	memset(val_str(0),0,strlen(val_str(0)));

	while (args--)
	    strcat(val_str(0),s);

	stack[top].type = TYPE_STR;
    }
    else if (type_int(0)) {
	int i = 0;
	while (args--)
	    val_str(0)[i++] = val_int(0);

	val_int(0) = 0;
	stack[top].type = TYPE_STR;
    }

    return 1;
}

int exec_command_sign(int args)
{
    if (!top)
	implicit_input(1,TYPE_INT);

    if (implicit && type_int(0)) {
	stack[top+1].val = val_int(0) < 0 ? -1 : val_int(0) == 0 ? 0 : 1;
	top++;
    }
    else if (!implicit) {
	stack[++top].val = -args;
    }

    return 1;
}

int exec_command_seq(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_INT);

    if (type_int(0)) {
	if (implicit)
	    args = val_int(0);

	int i, n = args;
	for (i = 0; i < n; i++)
	    stack[top+i].val = i+1;
	top += (i - 1);
    }

    return 1;
}

int exec_command_sum_stack(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_INT);

    if (implicit)
	args = top;

    int sum = 0;
    while (args--) {
	sum += val_int(0);
	zero(stack[top]);
	top--;
    }
    stack[++top].val = sum;

    return 1;
}

int exec_command_itoa(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_INT);

    if (type_int(0) && implicit) {
	itoa(val_int(0),val_str(0),10);
	stack[top].type = TYPE_STR;
    }
    else if (type_str(0) && implicit) {
	val_int(0) = atoi(val_str(0));
	stack[top].type = TYPE_INT;
    }
    else if (!implicit) {
	itoa(args,stack[top+1].val_str,10);
	stack[top+1].type = TYPE_STR;
	top++;
    }

    return 1;
}

int exec_command_whitespace(int args)
{
    if (implicit)
	args = 0;

    ssputchar(" \n"[args]);

    return 1;
}


int exec_command_consume(int args)
{
    if (implicit) {
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
	    strcpy(val_str(0),s);
	    stack_realloc();
	}
    }

    return 1;
}

int exec_command_log10(int args)
{
    if (implicit && !top)
	implicit_input(1,TYPE_FLT);

    if (type_int(0) && implicit) {
	float x = log10(val_int(0));

	if (!val_int(0))
	    x = 0;

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }
    else if (stack[top].type == TYPE_FLT && implicit) {
	float x = log10(val_flt(0));

	if (!val_flt(0))
	    x = 0;

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }
    else if (args) {
	float x = log10(args);

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }

    return 1;
}

int exec_command_xor(int args)
{
    if (!top && args)
	implicit_input(2,TYPE_INT);

    if (type_int(-1) && type_int(0) && implicit) {
	int x = val_int(-1) ^ val_int(0);
	val_int(-1) = x;
	zero(stack[top]);
	top--;
    }

    return 1;
}

int exec_command_copy_to_mem(int args)
{
    args = (implicit) ? 0 : args;

    while (top <= args)
	implicit_input(1,TYPE_INT);

    memcpy(&mem,&stack[top-args],sizeof(struct _stack));

    return 1;
}

int exec_command_not(int args)
{
    if (!top && !args)
	implicit_input(1,TYPE_INT);

    if (type_int(0) && implicit) {
	int x = !val_int(0);
	val_int(0) = x;
    }
    else if (stack[top].type == TYPE_FLT && implicit) {
	int x = !val_flt(0);
	val_int(0) = x;
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
    if (implicit && !top)
	implicit_input(1,TYPE_FLT);

    if (type_int(0) && implicit) {
	float x = log2(val_int(0));

	if (!val_int(0))
	    x = 0;

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }
    else if (stack[top].type == TYPE_FLT && implicit) {
	float x = log2(val_flt(0));

	if (!val_flt(0))
	    x = 0;

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }
    else if (args) {
	float x = log2(args);

	val_flt(0) = x;
	stack[top].type = TYPE_FLT;
    }

    return 1;
}

int exec_command_iswhole(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_FLT);

    if (stack[top].type == TYPE_FLT && implicit) {
	int x = (floor(val_flt(0)) == ceil(val_flt(0))) && (val_flt(0) >= 0);

	stack[++top].val = x;
    }
    else if (type_int(0) && implicit) {
	if (!implicit) {
	    if (val_int(0) != args) {
		noprint = 1;
		return -1;
	    }
	} else {
	    if (!val_int(0)) {
		noprint = 1;
		return -1;
	    }
	}
    }

    return 1;
}

int exec_command_split(int args)
{
    if (!top && implicit) {
	implicit_input(1,TYPE_STR);
	implicit_input(1,TYPE_INT);
    }
    else if (top == 1 && args)
	implicit_input(1,TYPE_STR);

    if (type_str(-1) && type_int(0) && implicit) {
	args = val_int(0);
	zero(stack[top]);
	top--;
    }

    if (type_str(0) && args > 0) {
	int i, l = strlen(val_str(0));

	if (args >= l)
	    return 1;

	char s[STACK_STRING_SIZE] = {0};
	strcpy(s,val_str(0));
	memset(val_str(0),0,STACK_STRING_SIZE);

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
	if (implicit) {
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
		strcpy(val_str(0),s);
		stack[top].type = TYPE_STR;
		stack_realloc();
	    }
	}
    }

    int i;
    for (i = 1; i <= top; i++) {
	if (stack[i].type == TYPE_INT) {
	    if (implicit)
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
    if (!top && implicit)
	implicit_input(2,TYPE_STR);
    else if (top == 1 && implicit)
	implicit_input(1,TYPE_STR);

    if (type_str(-1) && type_str(0)) {
	stack[top+1].val = strcmp(val_str(-1),val_str(0));
	top++;
    }

    return 1;
}

int exec_command_push_bin(int args)
{
    if (!top && implicit)
	implicit_input(1,TYPE_INT);

    if (type_int(0) && implicit) {
	int x = val_int(0);
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
    if (val_int(0) != args) {
	printf("stack = %d  args = %d\n",val_int(0),args);
	do_skip(args);
    }

    return 1;
}

int exec_command_exit_thingy(int args)
{
    if (!implicit) {
	if (val_int(0) != args)
	    noprint = 1;
    } else {
	if (!val_int(0))
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

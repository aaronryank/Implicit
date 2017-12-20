#include "simplestack.h"
#include "commands.h"

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
            case 0xB4: ret = exec_command_decrement(args);   break; // ´
	    case 0xB5: ret = exec_command_print_stack(args); break; // µ
	    case 0xB6: ret = exec_command_loop_forever(args);break; // ¶
	    case 0xB7: ret = exec_command_xor(args);         break; // ·
	    case 0xBD: ret = exec_command_log2(args);        break; // ½
	    case 0xC0: ret = exec_command_alpha(args);       break; // À
	    case 0xC1: ret = exec_command_log10(args);       break; // Á
	    case 0xD7: ret = exec_command_split(args);       break; // ×
	    case 0xD8: ret = exec_command_strcmp(args);      break; // Ø
	    case 0xDE: ret = exec_command_sum_stack(args);   break; // Þ
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
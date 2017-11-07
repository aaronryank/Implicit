# SimpleStack [![Build Status](https://travis-ci.org/aaronryank/simplestack.svg?branch=master)](https://travis-ci.org/aaronryank/simplestack)

SimpleStack, a.k.a. Implicit, is a simple stack-based programming language capable of solving trivial tasks in a short amount of bytes. It tries its best to guess the programmer's intentions, leading to some very short code.

# Syntax

Syntax is based off of [Cubically](//git.io/Cubically)'s - non-digit commands set the command, digit commands call them. For example, `x12` calls the command `x` with the argument 12.

If no arguments are passed to commands, they are called implicitly and have different behavior than if they were called with arguments.

The degree sign, &deg;, can be passed as an argument to a command and will mean "use the top stack value as an argument to this command".

# Commands

Commands have different behavior based on what is on the top of the stack.

int/float

|command|argument x|implicit|implicit input|
|-|-|-|-|
|`+`|add x to top of stack|pop top two stack values, add, push result|two integers|
|`-`|subtract x from top of stack|pop top two stack values, subtract, push result. if top two values are not int, decrement|two integers|
|`/`|divide top of stack by x|pop top two stack values, divide, push result|two integers|
|`*`|multiply top of stack by x|pop top two stack values, multiply, push result|two integers|
|`_`|modulo top of stack by x|pop top two stack values, modulo, push result|two integers|
|`^`|raise top of stack by x|raise top of stack by 2|integer|
|`>`|push top stack value > x|push previous stack value > top stack value|two integers|
|`<`|push top stack value < x|push previous stack value < top stack value|two integers|
|`\\`|UNUSED|invert sign of top of stack|string|
|`.`|UNUSED|fail-safe increment top of stack|integer|
|`¦`|if x is 1, change to lowercase, if x is 2, change to uppercase|swap case|string|
|`¸`|UNUSED|fail-safe decrement top of stack|string|
|`é`|exit with implicit output if top of stack == x|exit with implicit output if top of stack truthy|string|
|`è`|exit without implicit output if top of stack == x|exit without implicit output if top of stack truthy|string|
|`±`|push negative x|push sign of top of stack (-1 for negative, 0 for 0, 1 for positive)|integer|
|`¡`|push 1..x|x = top of stack|integer|
|`ì`|push x as string|convert top of stack to string|integer|
|`½`|push log2(x)|top of stack = log2(top of stack)|float|
|`Á`|push log10(x)|top of stack = log10(top of stack)|float|
|`ö`|UNUSED|UNUSED|float|

string

|command|argument x|implicit|implicit input|
|-|-|-|-|
|`+`|concatenate top of stack (char or string) to previous stack string x times|concatenate top of stack (char or string) to previous stack string|two integers|
|`-`|cut x characters off of top of stack|cut last character off top of stack|two integers|
|`*`|insert top of stack at xth position of previous stack string, pop|insert top of stack at beginning of previous stack string, pop|two integers|
|`/`|same as `*` but overwrites characters|same as `*` but overwrites characters|two integers|
|`_`|push xth character in top of stack to stack|push last character in top of stack to stack|two integers|
|`^`|cut all but the first x characters out of the top of stack|push length of string on top of stack|integer|
|`>`|push string length of top stack value > x|push string length of previous stack value > string length of top stack value|two integers|
|`<`|push string length of top stack value < x|push string length of previous stack value < string length of top stack value|two integers|
|`\\`|chop x characters off front of string|reverse string|string|
|`.`|UNUSED|UNUSED|integer|
|`¦`|if x is 1, change to lowercase, if x is 2, change to uppercase|swap case of all characters in top stack string|string|
|`¸`|UNUSED|pop string, push each character individually|string|
|`é`|increment each character in string by x (e.g. x = 1: a becomes b, * becomes +, etc)|x = 1|string|
|`è`|decrement each character in string by x (e.g. x = 1: b becomes a, + becomes *, etc)|x = 1|string|
|`±`|push negative x|UNUSED|integer|
|`¡`|push 1..x|UNUSED|integer|
|`ì`|push x as string|convert top stack string to integer|integer|
|`ö`|UNUSED|UNUSED|float|
|`×`|split top stack string into chunks of x|split second-to-top stack string into chunks of top stack int|string|

float

|command|argument x|implicit|implicit input|
|-|-|-|-|
|`½`|push log2(x)|top of stack = log2(top of stack)|float|
|`ö`|UNUSED|push is-whole(top of stack)|float|
|`Á`|push log10(x)|top of stack = log10(top of stack)|float|

any

|command|argument x|implicit|implicit input|
|-|-|-|-|
|`%`|print x as integer|print top of stack as default type|
|`@`|print x as ASCII|print top of stack as ASCII (null byte if non-integer)|
|`&`|UNUSED|exit program without implicit output|
|`~`|read x characters, push to stack|read character, push to stack|
|`$`|read x integers, push to stack|read integer, push to stack|
|`'`|read x newline-delimited strings, push to stack| read newline-delimited string, push to stack|
|`:`|push x to stack|duplicate top of stack|integer|
|`#`|pop all but first x values on stack|push length of stack|
|`,`|swap the xth stack value with the top stack value|swap top two stack values|
|`=`|compare top stack value with x for equality, push result|compare top two stack values for equality, push result|
|`"`|turn top x stack values into string|turn stack into string|
|`;`|pop x values from stack|pop top of stack|
|`[`|UNUSED|pop top of stack into register|
|`]`|UNUSED|pull register onto stack|
|`§`|duplicate top of stack x times, perform `"x`|perform `"1`|
|`` ` ``|print the top x stack values (0 to print the entire stack)|debugging info (print type of top of stack)|
|`Þ`|pop and sum the top x stack values|pop and sum the entire stack|integer|
|`ß`|1- print newline|"space" print a space|
|`¯`|equivalent to `[x]`|equivalent to `[]`|
|`©`|consume all input as integers if x == 1 and strings if x == 2, push to stack|consume all input as ASCII, push to stack|all|

# Guesswork

SimpleStack guesses the programmer's intentions in an attempt to make golfier code. When called implicitly, different functions have different effects based on the stack.

# `+`, `-`

If the top two stack values are [INT1 INT2], it will pop them and push [INT1+INT2] (or [INT1-INT2] for `-`).  
If the top stack value is an integer and the stack length is 1, it will increment (or decrement for `-`) the integer. `+1` and `.` can be used to increment the top integer when the stack length is not 1. `-1` and `¸` can be used to decrement the top integer when the stack length is not 1.  
If the top two stack values are [STRING INT], it will have the default string behavior.

# `*`, `/`

If the top two stack values are [INT1 INT2], it will pop them and push [INT1*INT2] (or [INT1/INT2] for `/`).  
If the top stack value is an integer and the stack length is 1, it won't do anything.  

# Examples

Hello, World!

    «Hello, World!
    »

Hello, World!

    Hello:44:32World:33:10"

cat

    ~+(-@~+)&

string length

    '^

add input

    +

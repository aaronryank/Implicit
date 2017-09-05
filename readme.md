# SimpleStack

SimpleStack, a.k.a. Implicit, is a simple stack-based programming language capable of solving trivial tasks in a short amount of bytes. It features implicit input, implicit output, and calling commands with implicit arguments.

Its syntax is based off of [Cubically](//git.io/Cubically)'s.

# Commands

int/float

|command|argument x|implicit|
|-|-|-|
|`+`|add x to top of stack|pop top two stack values, add, push result|
|`-`|subtract x from top of stack|pop top two stack values, subtract, push result. if top two values are not int, decrement|
|`/`|divide top of stack by x|pop top two stack values, divide, push result|
|`*`|multiply top of stack by x|pop top two stack values, multiply, push result|
|`_`|top of stack equals top of stack mod x|pop top two stack values, modulo, push result|
|`^`|raise top of stack by x|raise top of stack by 2|
|`>`|push top stack value > x|push previous stack value > top stack value|
|`<`|push top stack value < x|push previous stack value < top stack value|
|`\\`|UNUSED|invert sign of top of stack|

string

|command|argument x|implicit|
|-|-|-|
|`+`|concatenate top of stack (char or string) to previous stack string x times|concatenate top of stack (char or string) to previous stack string|
|`-`|cut x characters off of top of stack|cut last character off top of stack|
|`*`|insert top of stack at xth position of previous stack string, pop|insert top of stack at beginning of previous stack string, pop|
|`/`|same as `*` but overwrites characters|same as `*` but overwrites characters|
|`_`|push xth character in top of stack to stack|push last character in top of stack to stack|
|`^`|cut all but the first x characters out of the top of stack|push length of string on top of stack|
|`>`|push string length of top stack value > x|push string length of previous stack value > string length of top stack value|
|`<`|push string length of top stack value < x|push string length of previous stack value < string length of top stack value|
|`\\`|UNUSED|reverse top of stack|

any

|command|argument x|implicit|
|-|-|-|
|`%`|print x as integer|print top of stack as default type|
|`@`|print x as ASCII|print top of stack as ASCII (null byte if non-integer)|
|`&`|exit program if x is truthy|exit program|
|`~`|read x characters, push to stack|read character, push to stack|
|`$`|read x integers, push to stack|read integer, push to stack|
|`'`|read x newline-delimited strings, push to stack| read newline-delimited string, push to stack|
|`:`|push x to stack|duplicate top of stack|
|`#`|pop all but first x values on stack|push length of stack|
|`,`|swap the xth stack value with the top stack value|swap top two stack values|
|`=`|compare top stack value with x for equality, push result|compare top two stack values for equality, push result|
|`"`|turn top x stack values into string|turn stack into string|
|`:`|push x to stack|duplicate top of stack|
|`;`|pop x values from stack|pop top of stack|
|`[`|UNUSED|pop top of stack into register|
|`]`|UNUSED|pull register onto stack|

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

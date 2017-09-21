cat <<<$1 >interp.c
cat >source.ss
gcc interp.c -o interp -lm
./interp u source.ss <<<""

# Gets all files that end in .c
files="$(find . -type f | grep -vE '\./\.git|\.gch' | grep '\.c' | sort -r | sed -n -e 'H;${x;s/\n/ /g;s/^,//;p;}')"

warnings="-Wextra -Wall -pedantic -Wno-implicit-fallthrough"
name="expr"
opt="2"
std="c17"
linkage="-lm -lreadline"
extra_flags=""
# Will pass args to gcc
exit `eval "gcc $files $warnings -std=$std -O$opt -o $name.out $linkage $extra_flags $*"`
rm uncrustify.lst 2> /dev/null
find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "./testbed/examples/Fake6502/*" -not -path "./testbed/plugins/LLDB/*" > uncrustify.lst
uncrustify -c uncrustify.cfg --no-backup -F uncrustify.lst
rm uncrustify.lst 2> /dev/null
find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "./examples/fake_6502/*" -not -path "./src/plugins/lldb/*" -not -path "./src/external/*" -not -path "./old/*" > uncrustify.lst
bin/macosx/uncrustify -c scripts/uncrustify.cfg --no-backup -F uncrustify.lst

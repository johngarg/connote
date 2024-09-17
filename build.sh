#!/usr/bin/env sh

gcc -o bin/note src/*.c;

./bin/note file --title "Hello there" --k test1 test2 --sig 12=2
# ./bin/note file test.md --title "Hello there" --k test1 test2 --sig 12=2

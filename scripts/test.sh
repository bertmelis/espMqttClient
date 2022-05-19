#!/bin/sh

pio test -e native -f outbox
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./.pio/build/native/program

pio test -e native -f packets
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s -v ./.pio/build/native/program

pio test -e native -f parser
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./.pio/build/native/program

pio test -e native -f remainingLength
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./.pio/build/native/program

pio test -e native -f string
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./.pio/build/native/program

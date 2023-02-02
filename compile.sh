#!/bin/bash

if ! [[ -d "build" ]]
then
    mkdir build
fi

if ! [[ -d "build/obj" ]]
then
    mkdir build/obj
fi

gcc -c -O2 src/main.c -obuild/obj/main.o -Iinclude
gcc -c -O2 src/CB/cb_buffer.c -obuild/obj/cb_buffer.o -Iinclude

gcc build/obj/main.o build/obj/cb_buffer.o -obuild/le

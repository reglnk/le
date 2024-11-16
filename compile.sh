#!/bin/sh

if [ ! -d "build" ]
then
    mkdir build
fi

if [ ! -d "build/obj" ]
then
    mkdir build/obj
fi

if [ -z "$CC" ]; then
	if [ -n "$(command -v clang)" ]; then
		CC=clang
	elif [ -n "$(command -v gcc)" ]; then
		CC=gcc
	else
		CC=cc
	fi
fi

echo compiler: $CC

$CC -c -O2 src/main.c -obuild/obj/main.o -Iinclude
$CC -c -O2 src/CB/cb_buffer.c -obuild/obj/cb_buffer.o -Iinclude

$CC build/obj/main.o build/obj/cb_buffer.o -obuild/le

#!/bin/sh

if [ ! -f "build/le" ]
then
    ./compile.sh
fi

cp build/le /usr/local/bin


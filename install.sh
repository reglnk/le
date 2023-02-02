#!/bin/bash

if ! [[ -f "build/le" ]]
then
    ./compile.sh
fi

sudo cp build/le /usr/local/bin


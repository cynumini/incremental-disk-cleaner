#!/usr/bin/env sh
set -e

mkdir -p ./out
gcc ./src/*.c -o ./out/idc -std=c99 -Wall -Werror -g -pedantic
./out/idc $@

#!/usr/bin/env sh

gcc ./src/main.c -o ./out/idc
./out/idc $@

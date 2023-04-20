#!/usr/bin/env bash

flex lex.l
cc -o mgen lex.yy.c -ll


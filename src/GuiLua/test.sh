#! /bin/bash

wd=$(pwd)

export LUA_PATH="$wd/../../lib/?.lua;./?.lua"
../../skang -l test -foo "argy bargy"

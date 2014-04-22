#! /bin/bash

wd=$(pwd)

export LUA_PATH="$wd/../../libraries/?.lua;./?.lua"
../../skang -l test -foo "argy bargy"

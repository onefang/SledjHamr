#! /bin/bash

wd=$(pwd)

export LUA_PATH="$wd/lib/?.lua;$wd/src/GuiLua/?.lua"
export LUA_CPATH="$wd/lib/lib?.so;$wd/src/GuiLua/?.so"
./extantz

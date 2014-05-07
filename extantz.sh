#! /bin/bash

wd=$(pwd)

export LUA_PATH="$wd/src/GuiLua/?.lua"
export LUA_CPATH="$wd/src/GuiLua/?.so"
./extantz

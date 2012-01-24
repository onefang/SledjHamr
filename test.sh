#! /bin/bash

reset

wd=$(pwd)

./build.sh || exit

echo "_______________ TESTING LuaSL _______________"
cd $wd/LuaSL/testLua
export LUA_SOPATH='../../libraries/luaproc/' 
export LD_LIBRARY_PATH="../../libraries/luajit-2.0/src:$LD_LIBRARY_PATH"
../LuaSL


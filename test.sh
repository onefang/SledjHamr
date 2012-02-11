#! /bin/bash

reset

wd=$(pwd)

./build.sh || exit

echo "_______________ TESTING LuaSL _______________"
# Kill any left overs.
killall -KILL LuaSL
cd $wd/LuaSL/testLua
export LUA_PATH="$wd/LuaSL/src/?.lua"
export LUA_SOPATH='../../libraries/luaproc/'
export LD_LIBRARY_PATH="../../libraries/luajit-2.0/src:$LD_LIBRARY_PATH"
export EINA_LOG_LEVELS="ecore:2,ecore_con:2"

case $@ in

    ddd)
	ddd ../LuaSL
	;;

    gdb)
	gdb ../LuaSL
	;;

    *)
	../LuaSL &
	sleep 1
	../LuaSL_test
	;;

esac


#! /bin/bash

reset

wd=$(pwd)

./build.sh || exit

echo "_______________ TESTING extantz _______________"
cd $wd/ClientHamr/extantz
./extantz &
sleep 1

echo "_______________ TESTING LuaSL _______________"
# Kill any left overs.
killall -KILL LuaSL
cd $wd/LuaSL/testLua
export LUA_PATH="$wd/LuaSL/src/?.lua"
export LUA_SOPATH='../../libraries/luaproc/'
export LD_LIBRARY_PATH="../../libraries/luajit-2.0/src:$LD_LIBRARY_PATH"
export EINA_LOG_LEVELS="eo:2,ecore:2,ecore_con:2"

case $@ in

    ddd)
	ddd ../LuaSL
	;;

    gdb)
	gdb ../LuaSL
	;;

    *)
	echo "_______________ STARTING LuaSL _______________"
	../LuaSL &
	sleep 1
	echo "_______________ STARTING LuaSL_test _______________"
	../LuaSL_test
	;;

esac


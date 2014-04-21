#! /bin/bash

wd=$(pwd)

# Kill any left overs.
killall -KILL LuaSL
cd $wd/testLua
export LUA_PATH="$wd/src/?.lua"

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

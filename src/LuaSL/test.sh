#! /bin/bash

wd=$(pwd)

# Kill any left overs.
killall -KILL LuaSL
export LUA_PATH="$wd/../../libraries/?.lua"

case $@ in

    ddd)
	ddd ../../LuaSL
	;;

    gdb)
	gdb ../../LuaSL
	;;

    *)
	echo "_______________ STARTING LuaSL _______________"
	../../LuaSL &
	sleep 1
	echo "_______________ STARTING LuaSL_test _______________"
	./LuaSL_test
	;;

esac

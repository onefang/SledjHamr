#! /bin/bash

reset

wd=$(pwd)

./build.lua || exit

echo "_______________ TESTING extantz _______________"
./extantz.sh &
sleep 1

echo "_______________ TESTING GuiLua _______________"
cd $wd/src/GuiLua
./test.sh &
sleep 1

echo "_______________ TESTING love and LuaSL _______________"
cd $wd
./love $0

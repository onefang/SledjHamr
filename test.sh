#! /bin/bash

reset

wd=$(pwd)

./build.lua || exit

echo "_______________ TESTING extantz _______________"
./extantz &
sleep 1

echo "_______________ TESTING GuiLua _______________"
cd $wd/src/GuiLua
./test.sh &
sleep 1

echo "_______________ TESTING LuaSL _______________"
cd $wd/src/LuaSL
./test.sh $0

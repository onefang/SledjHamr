#! /bin/bash

reset

wd=$(pwd)

./build.lua || exit

echo "_______________ TESTING extantz, love and LuaSL _______________"
./extantz &
sleep 1

echo "_______________ TESTING GuiLua _______________"
cd $wd/src/GuiLua
./test.sh &

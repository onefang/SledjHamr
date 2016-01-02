#! /bin/bash

reset

wd=$(pwd)

./killem.sh
sleep 1

./build.lua || exit

echo "_______________ TESTING extantz, love and LuaSL _______________"
./LuaSL & sleep 2 && ./love & sleep 1 && ./extantz &
sleep 30

echo "_______________ TESTING GuiLua _______________"
cd $wd/src/GuiLua
./test.sh &

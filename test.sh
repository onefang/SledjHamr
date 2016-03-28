#! /bin/bash

reset

wd=$(pwd)

./build.lua || exit

echo "_______________ TESTING extantz, love and LuaSL _______________"
bin/LuaSL & sleep 2 && bin/love & sleep 1 && bin/extantz &
sleep 30

echo "_______________ TESTING GuiLua _______________"
cd $wd/src/GuiLua
./test.sh &

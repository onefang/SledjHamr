#! /bin/bash

reset

wd=$(pwd)

./build.lua || exit

echo "_______________ TESTING extantz _______________"
cd $wd/ClientHamr/extantz
./test.sh &
sleep 1

echo "_______________ TESTING GuiLua _______________"
cd $wd/ClientHamr/GuiLua
./test.sh &
sleep 1

echo "_______________ TESTING LuaSL _______________"
cd $wd/LuaSL
./test.sh $0

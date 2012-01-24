#! /bin/bash

reset

wd=$(pwd)

./build.sh || exit

echo "_______________ TESTING LuaSL _______________"
cd $wd/LuaSL
./LuaSL


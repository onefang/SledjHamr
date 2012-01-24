#! /bin/bash

wd=$(pwd)

echo "_______________ BUILDING LuaJIT _______________"
cd libraries/luajit-2.0
make


echo "_______________ BUILDING luaproc _______________"
cd $wd/libraries/luaproc
make


echo "_______________ BUILDING lemon _______________"
cd $wd/libraries/lemon
rm -f *.o lemon

CFLAGS="-g -Wall -I include $CFLAGOPTS"
LDFLAGS="-L lib -L /usr/lib -L /lib"
LFLAGS="-d"

names="lemon"
objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o ../$i.o $i.c"
    echo $command
    $command
    objects="$objects ../$i.o"
done
command="gcc $CFLAGS -o lemon $objects $LDFLAGS $libs"
echo $command
$command


echo "_______________ BUILDING eina _______________"
cd $wd/libraries/eina


echo "_______________ BUILDING eet _______________"
cd $wd/libraries/eet


echo "_______________ BUILDING evas _______________"
cd $wd/libraries/evas


echo "_______________ BUILDING ecore _______________"
cd $wd/libraries/ecore


echo "_______________ BUILDING embryo _______________"
cd $wd/libraries/embryo


echo "_______________ BUILDING edje _______________"
cd $wd/libraries/edje


echo "_______________ BUILDING LuaSL _______________"
cd $wd/LuaSL
./build.sh



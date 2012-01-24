#! /bin/bash

wd=$(pwd)

echo "_______________ BUILDING LuaJIT _______________"
cd $wd/libraries/luajit-2.0
make amalg PREFIX=$()/src


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
    command="gcc $CFLAGS -c -o $i.o $i.c"
    echo $command
    $command
    objects="$objects $i.o"
done
command="gcc $CFLAGS -o lemon $objects $LDFLAGS $libs"
echo $command
$command


# Test if edje is already available, build EFL up to edje if not.
# TODO - Check EFL version.
hash edje_cc 2>&- || {

    echo "_______________ BUILDING eina _______________"
    cd $wd/libraries/eina
    ./configure && make


    echo "_______________ BUILDING eet _______________"
    cd $wd/libraries/eet
    ./configure && make


    echo "_______________ BUILDING evas _______________"
    cd $wd/libraries/evas
    ./configure && make


    echo "_______________ BUILDING ecore _______________"
    cd $wd/libraries/ecore
    ./configure && make


    echo "_______________ BUILDING embryo _______________"
    cd $wd/libraries/embryo
    ./configure && make


    echo "_______________ BUILDING edje _______________"
    cd $wd/libraries/edje
    ./configure && make


# TODO - Install this EFL version, and/or get the rest of the system to use it.
}


echo "_______________ BUILDING LuaSL _______________"
cd $wd/LuaSL
./build.sh



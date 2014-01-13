#! /bin/bash

wd=$(pwd)

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

echo "_______________ BUILDING Irrlicht _______________"
cd $wd/libraries/irrlicht-1.8.1/source/Irrlicht 
make

echo "_______________ BUILDING LuaSL _______________"
cd $wd/LuaSL
./build.sh

echo "_______________ BUILDING extantz _______________"
cd $wd/ClientHamr/extantz
./build.sh

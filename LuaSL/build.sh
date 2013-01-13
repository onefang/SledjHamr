#! /bin/bash


export LOCALDIR=`pwd`

cd src
rm -f ../LuaSL *.o *.output *.backup ../luac.out ../*.edj LuaSL_lexer.h LuaSL_lexer.c LuaSL_lemon_yaccer.h LuaSL_lemon_yaccer.c LuaSL_lemon_yaccer.out


# This assumes you have EFL installed in one of two standard places.
if [ -d "/opt/e17" ]
then
    export E17DIR="/opt/e17"
else
    export E17DIR="/usr"
fi

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.

CFLAGS="-g -Wall -Wunreachable-code -I include -I $LOCALDIR/src"
CFLAGS="$CFLAGS -I ../../libraries"
CFLAGS="$CFLAGS -I ../../libraries/luajit-2.0/src"
#CFLAGS="$CFLAGS -I /usr/include/lua5.1"
CFLAGS="$CFLAGS -I $E17DIR/include/eo-1"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1/eina"
CFLAGS="$CFLAGS -I $E17DIR/include/eet-1"
CFLAGS="$CFLAGS -I $E17DIR/include/embryo-1"
CFLAGS="$CFLAGS -I $E17DIR/include/edje-1"
CFLAGS="$CFLAGS -I $E17DIR/include/evas-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-1"
CFLAGS="$CFLAGS -I $E17DIR/include"
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

LDFLAGS="-L ../../libraries/luajit-2.0/src -L lib -L /usr/lib -L /lib -L $E17DIR/lib"
libs="-leo -lecore -levas -ledje -lembryo -leet -leina -lluajit -lpthread -lm"
#LDFLAGS="-L /usr/lib/lua/5.1 -L lib -L /usr/lib -L /lib -L $E17DIR/lib"
#libs="-lecore -levas -ledje -lembryo -leet -leina -llua5.1 -lpthread -lm"
# These need to be added to libs if linking staticaly, though some parts of EFL don't like that.
#-lecore_evas \
#-lecore_file \
#-ldl \
#-lfontconfig \
#-lfreetype \
#-lexpat \
#-lrt \
#-lz

LFLAGS="-d"
EDJE_FLAGS="-id images -fd fonts"
LD_RUN_PATH="../../libraries/luajit-2.0/src:"


# Run lemon first, flex depends on it to define the symbol values.
command="../../libraries/lemon/lemon -s -T../../libraries/lemon/lempar.c LuaSL_lemon_yaccer.y"
echo $command
$command

command="flex -C --outfile=LuaSL_lexer.c --header-file=LuaSL_lexer.h LuaSL_lexer.l"
echo $command
$command

command="edje_cc $EDJE_FLAGS LuaSL.edc ../LuaSL.edj"
echo $command
$command

names="LuaSL_main LuaSL_compile LuaSL_threads LuaSL_utilities LuaSL_lexer LuaSL_lemon_yaccer"
objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o $i.o $i.c"
    echo $command
    $command
    objects="$objects $i.o"
done
command="gcc $CFLAGS -o ../LuaSL $objects $LDFLAGS $libs"
echo $command
$command

names="LuaSL_test LuaSL_utilities"
objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o $i.o $i.c"
    echo $command
    $command
    objects="$objects $i.o"
done
command="gcc $CFLAGS -o ../LuaSL_test $objects $LDFLAGS $libs"
echo $command
$command


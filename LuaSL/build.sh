#! /bin/bash

export LOCALDIR=`pwd`

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.

CFLAGS="-g -Wall -Wunreachable-code -I include -I $LOCALDIR"
CFLAGS="$CFLAGS -I ../../libraries"
CFLAGS="$CFLAGS $(pkg-config --cflags luajit)"
CFLAGS="$CFLAGS $(pkg-config --cflags eo)"
CFLAGS="$CFLAGS $(pkg-config --cflags eet)"
CFLAGS="$CFLAGS $(pkg-config --cflags ecore-con)"
CFLAGS="$CFLAGS $(pkg-config --cflags ecore-evas)"
CFLAGS="$CFLAGS $(pkg-config --cflags ecore-file)"
CFLAGS="$CFLAGS $(pkg-config --cflags edje)"
CFLAGS="$CFLAGS -DPACKAGE_BIN_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_LIB_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

LDFLAGS="$(pkg-config --libs-only-L luajit) -L lib -L /usr/lib -L /lib"
libs="$(pkg-config --libs edje) $libs $(pkg-config --libs luajit)"

LFLAGS="-d"
EDJE_FLAGS="-id images -fd fonts"
# Dunno why I needed this, not gonna work with a packaged LuaJIT anyway.
#LD_RUN_PATH="../../libraries/LuaJIT-2.0.2/src:"

cd src
echo "clean"
rm -f ../LuaSL *.o *.output *.backup ../luac.out ../*.edj LuaSL_lexer.h LuaSL_lexer.c LuaSL_lemon_yaccer.h LuaSL_lemon_yaccer.c LuaSL_lemon_yaccer.out

# Run lemon first, flex depends on it to define the symbol values.
command="../../libraries/lemon/lemon -s -T../../libraries/lemon/lempar.c LuaSL_lemon_yaccer.y"
echo "lemon"
$command

command="flex -C --outfile=LuaSL_lexer.c --header-file=LuaSL_lexer.h LuaSL_lexer.l"
echo "flex"
$command

command="edje_cc $EDJE_FLAGS LuaSL.edc ../LuaSL.edj"
echo "edje_cc"
$command

names="LuaSL_main LuaSL_compile LuaSL_threads LuaSL_utilities LuaSL_lexer LuaSL_lemon_yaccer"
objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o $i.o $i.c"
    echo $i
    $command
    objects="$objects $i.o"
done
command="gcc $CFLAGS -o ../LuaSL $objects $LDFLAGS $libs"
echo "LuaSL"
$command

names="LuaSL_test LuaSL_utilities"
objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o $i.o $i.c"
    echo $i
    $command
    objects="$objects $i.o"
done
command="gcc $CFLAGS -o ../LuaSL_test $objects $LDFLAGS $libs"
echo "LuaSL_test"
$command

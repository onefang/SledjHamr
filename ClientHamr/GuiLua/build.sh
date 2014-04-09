#! /bin/bash

export LOCALDIR=`pwd`

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.

# This assumes you have EFL installed in one of two standard places.
if [ -d "/opt/EFL" ]
then
    export E17DIR="/opt/EFL"
else
    export E17DIR="/usr"
fi

CFLAGS="-g -Wall -I include -I $LOCALDIR"
CFLAGS="$CFLAGS -I ../../libraries"
#CFLAGS="$CFLAGS -I ../../libraries/LuaJIT-2.0.2/src"
CFLAGS="$CFLAGS $(pkg-config --cflags luajit)"
#CFLAGS="$CFLAGS -I /usr/include/lua5.1"
CFLAGS="$CFLAGS -I $E17DIR/include/eo-1"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1/eina"
CFLAGS="$CFLAGS -I $E17DIR/include/eet-1"
CFLAGS="$CFLAGS -I $E17DIR/include/embryo-1"
CFLAGS="$CFLAGS -I $E17DIR/include/edje-1"
CFLAGS="$CFLAGS -I $E17DIR/include/evas-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-1"
CFLAGS="$CFLAGS -I $E17DIR/include/efl-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-con-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-evas-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-file-1"
CFLAGS="$CFLAGS -I $E17DIR/include"
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

#LDFLAGS="-L ../../libraries/LuaJIT-2.0.2/src -L lib -L /usr/lib -L /lib -L $E17DIR/lib"
#libs="-leo -lecore -levas -ledje -lembryo -leet -leina -lluajit -lpthread -lm"
LDFLAGS="-L $LOCALDIR $(pkg-config --libs-only-L luajit) -L lib -L /usr/lib -L /lib -L $E17DIR/lib"
libs="-leo -lecore -levas -ledje -lembryo -leet -leina $(pkg-config --libs luajit) -lpthread -lm -ldl"
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

echo "clean"
rm -f test_c.so GuiLua.o libGuiLua.so skang
echo "C modules"
gcc $CFLAGS -fPIC -shared -o test_c.so test_c.c
gcc $CFLAGS -fPIC -c GuiLua.c
echo "C libraries"
gcc $CFLAGS -shared -Wl,-soname,libGuiLua.so -o libGuiLua.so GuiLua.o
echo "C apps"
gcc $CFLAGS -Wl,-export-dynamic -o skang skang.c $LDFLAGS -lGuiLua $libs

#! /bin/bash

export LOCALDIR=`pwd`

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.


CFLAGS="-g -Wall -I include -I $LOCALDIR"
CFLAGS="$CFLAGS -I ../../libraries"
CFLAGS="$CFLAGS $(pkg-config --cflags luajit)"
CFLAGS="$CFLAGS $(pkg-config --cflags elementary)"
CFLAGS="$CFLAGS -DPACKAGE_BIN_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_LIB_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

LDFLAGS="-L $LOCALDIR $(pkg-config --libs-only-L luajit) -L lib -L /usr/lib -L /lib"
libs="$(pkg-config --cflags --libs elementary) $(pkg-config --libs luajit) -lpthread -lm -ldl"

echo "clean"
rm -f test_c.so GuiLua.o libGuiLua.so skang
echo "C modules"
gcc $CFLAGS -fPIC -shared -o test_c.so test_c.c
gcc $CFLAGS -fPIC -c GuiLua.c
echo "C libraries"
gcc $CFLAGS -shared -Wl,-soname,libGuiLua.so -o libGuiLua.so GuiLua.o
ln -fs libGuiLua.so widget.so
echo "C apps"
gcc $CFLAGS -Wl,-export-dynamic -o skang skang.c $LDFLAGS -lGuiLua $libs

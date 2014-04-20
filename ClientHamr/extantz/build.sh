#! /bin/bash

export LOCALDIR=`pwd`

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.

CFLAGS="-g -Wall -I include -I $LOCALDIR"
CFLAGS="$CFLAGS -I ../../libraries -I../../libraries/irrlicht-1.8.1/include -I/usr/X11R6/include"
CFLAGS="$CFLAGS $(pkg-config --cflags luajit)"
CFLAGS="$CFLAGS $(pkg-config --cflags elementary)"
CFLAGS="$CFLAGS $(pkg-config --cflags ephysics)"
CFLAGS="$CFLAGS -DPACKAGE_BIN_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_LIB_DIR=\"$LOCALDIR\""
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

LDFLAGS="-L $LOCALDIR $(pkg-config --libs-only-L luajit) -L lib -L /usr/lib -L /lib -L../../libraries/irrlicht-1.8.1/lib/Linux "
libs="$(pkg-config --cflags --libs elementary) $(pkg-config --libs luajit) -lpthread -lm -ldl -lIrrlicht -lGL -lbz2"

echo "clean"
rm -f extantz crappisspuke.o CDemo.o extantzCamera.o extantz.edj
echo "edje"
edje_cc -id images extantz.edc extantz.edj
echo "Irrlicht"
#g++ -O3 -ffast-math crappisspuke.cpp -o crappisspuke -I../../libraries/irrlicht-1.8/include -I/usr/X11R6/include -L../../libraries/irrlicht-1.8/lib/Linux -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor && ./crappisspuke
g++ $CFLAGS -O3 -ffast-math -c crappisspuke.cpp -o crappisspuke.o $LDFLAGS
g++ $CFLAGS -O3 -ffast-math -c CDemo.cpp -o CDemo.o $LDFLAGS
echo "extantz"
g++ $CFLAGS -O3 -ffast-math -c extantzCamera.cpp -o extantzCamera.o $LDFLAGS
gcc $CFLAGS extantz.c crappisspuke.o CDemo.o extantzCamera.o -o extantz $LDFLAGS $libs && strip extantz

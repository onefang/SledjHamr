export PKG_CONFIG_PATH="/opt/e17/lib/pkgconfig"

echo "clean"
rm -f extantz crappisspuke.o CDemo.o extantzCamera.o extantz.edj
echo "edje"
edje_cc -id images extantz.edc extantz.edj
echo "Irrlicht"
#g++ -O3 -ffast-math crappisspuke.cpp -o crappisspuke -I../../libraries/irrlicht-1.8/include -I/usr/X11R6/include -L../../libraries/irrlicht-1.8/lib/Linux -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor && ./crappisspuke
g++ -O3 -ffast-math -c crappisspuke.cpp -o crappisspuke.o -I../../libraries/irrlicht-1.8/include -I/usr/X11R6/include  $(pkg-config --cflags elementary)
g++ -O3 -ffast-math -c CDemo.cpp -o CDemo.o -I../../libraries/irrlicht-1.8/include -I/usr/X11R6/include  $(pkg-config --cflags elementary)
echo "extantz"
g++ -O3 -ffast-math -c extantzCamera.cpp -o extantzCamera.o -I../../libraries/irrlicht-1.8/include -I/usr/X11R6/include  $(pkg-config --cflags elementary)
gcc -g -DPACKAGE_BIN_DIR="\"$(pwd)\"" -DPACKAGE_DATA_DIR="\"$(pwd)\"" extantz.c crappisspuke.o CDemo.o extantzCamera.o -o extantz $(pkg-config --cflags --libs eo) $(pkg-config --cflags --libs ecore-x) $(pkg-config --cflags --libs elementary) $(pkg-config --cflags --libs ephysics)  -L../../libraries/irrlicht-1.8/lib/Linux -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor -lpng -ljpeg -lbz2 && strip extantz

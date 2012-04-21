gcc -g -DPACKAGE_DATA_DIR="\"$(pwd)\"" extantz.c -o extantz $(pkg-config --cflags --libs elementary) && ./extantz



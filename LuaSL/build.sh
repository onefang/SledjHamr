#! /bin/bash


export LOCALDIR=`pwd`

cd src

if [ -d "/opt/e17" ]
then
    export E17DIR="/opt/e17"
else
    export E17DIR="/usr"
fi

# No need for a make file, or dependencies, the entire thing takes only a few seconds to build.

CFLAGS="-g -Wall -I include -I $LOCALDIR/src"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1"
CFLAGS="$CFLAGS -I $E17DIR/include/eina-1/eina"
CFLAGS="$CFLAGS -I $E17DIR/include/eet-1"
CFLAGS="$CFLAGS -I $E17DIR/include/edje-1"
CFLAGS="$CFLAGS -I $E17DIR/include/evas-1"
CFLAGS="$CFLAGS -I $E17DIR/include/ecore-1"
CFLAGS="$CFLAGS -I $E17DIR/include"
CFLAGS="$CFLAGS -DPACKAGE_DATA_DIR=\"$LOCALDIR\" $CFLAGOPTS"

LDFLAGS="-L lib -L /usr/lib -L /lib -L $E17DIR/lib"
libs="-lecore -levas -ledje -leet -leina"
# These need to be added to libs if linking staticaly, though some part of EFL don't like that.
#-lecore_evas \
#-lecore_fb \
#-lecore_file \
#-lecore \
#-ledje \
#-levas \
#-lembryo \
#-leet \
#-leina \
#-llua \
#-lm \
#-ldl \
#-lglib-2.0 \
#-lpthread \
#-lfontconfig \
#-lfreetype \
#-lexpat \
#-lrt \
#-lz

names="LuaSL_main LuaSL_compile LuaSL_utilities"

EDJE_FLAGS="-id images -fd fonts"

rm -f ../LuaSL ../LuaSL_parser ../*.o *.output *.backup ../*.edj LuaSL_lexer.h LuaSL_lexer.c LuaSL_yaccer.h LuaSL_yaccer.tab.c
command="edje_cc $EDJE_FLAGS LuaSL.edc ../LuaSL.edj"
echo $command
$command

objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o ../$i.o $i.c"
    echo $command
    $command
    objects="$objects ../$i.o"
done

command="gcc $CFLAGS -o ../LuaSL $objects $LDFLAGS $libs"
echo $command
$command



names="LuaSL_LSL_tree LuaSL_lexer LuaSL_yaccer.tab"

LFLAGS="-d"

# Hmmm, we have a circular dependencie with the include fiels each of flex and btyacc generate.  So run btyacc twice.

# I want to remove -d, coz I want an enum, not a bunch of #defines, but btyacc creates #defines internally anyway.  sigh
command="btyacc -d -t -v -b LuaSL_yaccer -S btyacc-c.ske LuaSL_yaccer.y"
echo $command
$command

command="flex -C --outfile=LuaSL_lexer.c --header-file=LuaSL_lexer.h LuaSL_lexer.l"
echo $command
$command

# I want to remove -d, coz I want an enum, not a bunch of #defines, but btyacc creates #defines internally anyway.  sigh
command="btyacc -d -t -v -b LuaSL_yaccer -S btyacc-c.ske LuaSL_yaccer.y"
echo $command
$command


objects=""
for i in $names
do
    command="gcc $CFLAGS -c -o ../$i.o $i.c"
    echo $command
    $command
    objects="$objects ../$i.o"
done

command="gcc $CFLAGS -o ../LuaSL_parser $objects $LDFLAGS $libs"
echo $command
$command


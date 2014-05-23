#!/bin/bash

# Memory checker, and the default tool.
# --tool=memcheck --leak-check=full
#                 --track-origins=yes

# Cache and branch prediction profiler, analyse speed issues.
# --tool=cachegrind
#                 --branch-sim=yes

# Heap profiler, check memory sizes.
# --tool=massif

# Heap profiler.
# --tool=dhat

# "Call-graph generating cache profiler", complements cachegrind.
# --tool=callgrind

# Thread error detector.
# --tool=helgrind

# Thread error detector.
# --tool=drd

# "experimental tool that can detect overruns of stack and global arrays"
# --tool=sgcheck

tool="memcheck"
#tool="helgrind"
#tool="drd"
#extra=""
extra="--leak-check=full"

valgrind --tool=$tool --time-stamp=yes --log-file=valgrind_LuaSL.log   $extra ./LuaSL &
sleep 3
valgrind --tool=$tool --time-stamp=yes --log-file=valgrind_love.log    $extra ./love &
sleep 3
valgrind --tool=$tool --time-stamp=yes --log-file=valgrind_extantz.log $extra ./extantz


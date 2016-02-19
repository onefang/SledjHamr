#!/bin/bash

# TODO - Deal with the valgrind left overs, by scanning the output of the following lines, picking out the PIDs, then "kill -KILL pid"
ps aux | grep love
ps aux | grep LuaSL

killall -TERM love
sleep 1
killall -TERM LuaSL
sleep 1

ps aux | grep love
ps aux | grep LuaSL

killall -KILL love
sleep 1
killall -KILL LuaSL
sleep 1

ps aux | grep love
ps aux | grep LuaSL

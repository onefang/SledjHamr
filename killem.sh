#!/bin/bash

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

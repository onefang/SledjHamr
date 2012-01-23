#! /bin/bash

cd testLua

LUA_SOPATH="../../libraries/luaproc/" lua luaprocTest0.lua

time LUA_SOPATH="../../libraries/luaproc/" lua luaprocTest1.lua

time LUA_SOPATH="../../libraries/luaproc/" luajit luaprocTest1.lua

time LUA_SOPATH="../../libraries/luaproc/" lua luaprocTest2.lua

time LUA_SOPATH="../../libraries/luaproc/" luajit luaprocTest2.lua


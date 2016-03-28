#!/usr/bin/env luajit

local dir = ...

if 'nil' == type(dir) then
  local build, err = loadfile('../../build.lua')
  if build then
    setfenv(build, getfenv(2))
    build(2)
  else
    print("ERROR - " .. err)
  end
  dir = workingDir
end

removeFiles(dir, {'*.output', '*.backup', 'LuaSL_lexer.h', 'LuaSL_lexer.c', 'LuaSL_lemon_yaccer.h', 'LuaSL_lemon_yaccer.c', 'LuaSL_lemon_yaccer.out'})

-- Build and run lemon first, flex depends on it to define the symbol values.
compileFiles('lemon', dir .. '/../others/lemon', {'lemon'}, '')
runCommand('lemon',   dir, '../others/lemon/lemon -qs -T../others/lemon/lempar.c LuaSL_lemon_yaccer.y')
runCommand('flex',    dir, 'flex -C --outfile=LuaSL_lexer.c --header-file=LuaSL_lexer.h LuaSL_lexer.l')
compileFiles('../../LuaSL', dir, {'LuaSL_main', 'LuaSL_compile', 'LuaSL_lexer', 'LuaSL_lemon_yaccer'}, '')

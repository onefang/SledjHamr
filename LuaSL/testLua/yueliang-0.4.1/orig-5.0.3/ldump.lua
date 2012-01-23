--[[--------------------------------------------------------------------

  ldump.lua
  Save bytecodes in Lua
  This file is part of Yueliang.

  Copyright (c) 2005 Kein-Hong Man <khman@users.sf.net>
  The COPYRIGHT file describes the conditions
  under which this software may be distributed.

  See the ChangeLog for more information.

----------------------------------------------------------------------]]

--[[--------------------------------------------------------------------
-- Notes:
-- * LUA_NUMBER (double), byte order (little endian) and some other
--   header values hard-coded; see other notes below...
-- * One significant difference is that instructions are still in table
--   form (with OP/A/B/C/Bx fields) and luaP:Instruction() is needed to
--   convert them into 4-char strings
-- * Deleted:
--   luaU:DumpVector: folded into DumpLines, DumpCode
-- * Added:
--   luaU:endianness() (from lundump.c)
--   luaU:make_setS: create a chunk writer that writes to a string
--   luaU:make_setF: create a chunk writer that writes to a file
--     (lua.h contains a typedef for a Chunkwriter pointer, and
--      a Lua-based implementation exists, writer() in lstrlib.c)
--   luaU:from_double(x): encode double value for writing
--   luaU:from_int(x): encode integer value for writing
--     (error checking is limited for these conversion functions)
--     (double conversion does not support denormals or NaNs)
--   luaU:ttype(o) (from lobject.h)
----------------------------------------------------------------------]]

--requires luaP
luaU = {}

-- constants used by dumper
luaU.LUA_TNUMBER = 3 -- (all in lua.h)
luaU.LUA_TSTRING = 4
luaU.LUA_TNIL    = 0
luaU.LUA_TNONE  = -1

-- definitions for headers of binary files
luaU.LUA_SIGNATURE = "\27Lua"   -- binary files start with "<esc>Lua"
luaU.VERSION = 80               -- 0x50; last format change was in 5.0
luaU.VERSION0 = 80              -- 0x50; last major  change was in 5.0

-- a multiple of PI for testing native format
-- multiplying by 1E7 gives non-trivial integer values
luaU.TEST_NUMBER = 3.14159265358979323846E7

--[[--------------------------------------------------------------------
-- Additional functions to handle chunk writing
-- * to use make_setS and make_setF, see test_ldump.lua elsewhere
----------------------------------------------------------------------]]

------------------------------------------------------------------------
-- works like the lobject.h version except that TObject used in these
-- scripts only has a 'value' field, no 'tt' field (native types used)
------------------------------------------------------------------------
function luaU:ttype(o)
  local tt = type(o.value)
  if tt == "number" then return self.LUA_TNUMBER
  elseif tt == "string" then return self.LUA_TSTRING
  elseif tt == "nil" then return self.LUA_TNIL
  else
    return self.LUA_TNONE  -- the rest should not appear
  end
end

------------------------------------------------------------------------
-- create a chunk writer that writes to a string
-- * returns the writer function and a table containing the string
-- * to get the final result, look in buff.data
------------------------------------------------------------------------
function luaU:make_setS()
  local buff = {}
        buff.data = ""
  local writer =
    function(s, buff)  -- chunk writer
      if not s then return end
      buff.data = buff.data..s
    end
  return writer, buff
end

------------------------------------------------------------------------
-- create a chunk writer that writes to a file
-- * returns the writer function and a table containing the file handle
-- * if a nil is passed, then writer should close the open file
------------------------------------------------------------------------
function luaU:make_setF(filename)
  local buff = {}
        buff.h = io.open(filename, "wb")
  if not buff.h then return nil end
  local writer =
    function(s, buff)  -- chunk writer
      if not buff.h then return end
      if not s then buff.h:close(); return end
      buff.h:write(s)
    end
  return writer, buff
end

-----------------------------------------------------------------------
-- converts a IEEE754 double number to an 8-byte little-endian string
-- * luaU:from_double() and luaU:from_int() are from ChunkBake project
-- * supports +/- Infinity, but not denormals or NaNs
-----------------------------------------------------------------------
function luaU:from_double(x)
  local function grab_byte(v)
    return math.floor(v / 256),
           string.char(math.mod(math.floor(v), 256))
  end
  local sign = 0
  if x < 0 then sign = 1; x = -x end
  local mantissa, exponent = math.frexp(x)
  if x == 0 then -- zero
    mantissa, exponent = 0, 0
  elseif x == 1/0 then
    mantissa, exponent = 0, 2047
  else
    mantissa = (mantissa * 2 - 1) * math.ldexp(0.5, 53)
    exponent = exponent + 1022
  end
  local v, byte = "" -- convert to bytes
  x = mantissa
  for i = 1,6 do
    x, byte = grab_byte(x); v = v..byte -- 47:0
  end
  x, byte = grab_byte(exponent * 16 + x); v = v..byte -- 55:48
  x, byte = grab_byte(sign * 128 + x); v = v..byte -- 63:56
  return v
end

-----------------------------------------------------------------------
-- converts a number to a little-endian 32-bit integer string
-- * input value assumed to not overflow, can be signed/unsigned
-----------------------------------------------------------------------
function luaU:from_int(x)
  local v = ""
  x = math.floor(x)
  if x >= 0 then
    for i = 1, 4 do
      v = v..string.char(math.mod(x, 256)); x = math.floor(x / 256)
    end
  else-- x < 0
    x = -x
    local carry = 1
    for i = 1, 4 do
      local c = 255 - math.mod(x, 256) + carry
      if c == 256 then c = 0; carry = 1 else carry = 0 end
      v = v..string.char(c); x = math.floor(x / 256)
    end
  end
  return v
end

--[[--------------------------------------------------------------------
-- Functions to make a binary chunk
-- * many functions have the size parameter removed, since output is
--   in the form of a string and some sizes are implicit or hard-coded
-- * luaU:DumpVector has been deleted (used in DumpCode & DumpLines)
----------------------------------------------------------------------]]

------------------------------------------------------------------------
-- dump a block of literal bytes
------------------------------------------------------------------------
function luaU:DumpLiteral(s, D) self:DumpBlock(s, D) end

--[[--------------------------------------------------------------------
-- struct DumpState:
--   L  -- lua_State (not used in this script)
--   write  -- lua_Chunkwriter (chunk writer function)
--   data  -- void* (chunk writer context or data already written)
----------------------------------------------------------------------]]

------------------------------------------------------------------------
-- dumps a block of bytes
-- * lua_unlock(D.L), lua_lock(D.L) deleted
------------------------------------------------------------------------
function luaU:DumpBlock(b, D) D.write(b, D.data) end

------------------------------------------------------------------------
-- dumps a single byte
------------------------------------------------------------------------
function luaU:DumpByte(y, D)
  self:DumpBlock(string.char(y), D)
end

------------------------------------------------------------------------
-- dumps a 32-bit signed integer (for int)
------------------------------------------------------------------------
function luaU:DumpInt(x, D)
  self:DumpBlock(self:from_int(x), D)
end

------------------------------------------------------------------------
-- dumps a 32-bit unsigned integer (for size_t)
------------------------------------------------------------------------
function luaU:DumpSize(x, D)
  self:DumpBlock(self:from_int(x), D)
end

------------------------------------------------------------------------
-- dumps a LUA_NUMBER (hard-coded as a double)
------------------------------------------------------------------------
function luaU:DumpNumber(x, D)
  self:DumpBlock(self:from_double(x), D)
end

------------------------------------------------------------------------
-- dumps a Lua string
------------------------------------------------------------------------
function luaU:DumpString(s, D)
  if s == nil then
    self:DumpSize(0, D)
  else
    s = s.."\0"  -- include trailing '\0'
    self:DumpSize(string.len(s), D)
    self:DumpBlock(s, D)
  end
end

------------------------------------------------------------------------
-- dumps instruction block from function prototype
------------------------------------------------------------------------
function luaU:DumpCode(f, D)
  local n = f.sizecode
  self:DumpInt(n, D)
  --was DumpVector
  for i = 0, n - 1 do
    self:DumpBlock(luaP:Instruction(f.code[i]), D)
  end
end

------------------------------------------------------------------------
-- dumps local variable names from function prototype
------------------------------------------------------------------------
function luaU:DumpLocals(f, D)
  local n = f.sizelocvars
  self:DumpInt(n, D)
  for i = 0, n - 1 do
    self:DumpString(f.locvars[i].varname, D)
    self:DumpInt(f.locvars[i].startpc, D)
    self:DumpInt(f.locvars[i].endpc, D)
  end
end

------------------------------------------------------------------------
-- dumps line information from function prototype
------------------------------------------------------------------------
function luaU:DumpLines(f, D)
  local n = f.sizelineinfo
  self:DumpInt(n, D)
  --was DumpVector
  for i = 0, n - 1 do
    self:DumpInt(f.lineinfo[i], D)  -- was DumpBlock
  end
end

------------------------------------------------------------------------
-- dump upvalue names from function prototype
------------------------------------------------------------------------
function luaU:DumpUpvalues(f, D)
  local n = f.sizeupvalues
  self:DumpInt(n, D)
  for i = 0, n - 1 do
    self:DumpString(f.upvalues[i], D)
  end
end

------------------------------------------------------------------------
-- dump constant pool from function prototype
-- * nvalue(o) and tsvalue(o) macros removed
------------------------------------------------------------------------
function luaU:DumpConstants(f, D)
  local n = f.sizek
  self:DumpInt(n, D)
  for i = 0, n - 1 do
    local o = f.k[i]  -- TObject
    local tt = self:ttype(o)
    self:DumpByte(tt, D)
    if tt == self.LUA_TNUMBER then
      self:DumpNumber(o.value, D)
    elseif tt == self.LUA_TSTRING then
      self:DumpString(o.value, D)
    elseif tt == self.LUA_TNIL then
    else
      --lua_assert(0)  -- cannot happen
    end
  end
  n = f.sizep
  self:DumpInt(n, D)
  for i = 0, n - 1 do
    self:DumpFunction(f.p[i], f.source, D)
  end
end

------------------------------------------------------------------------
-- dump child function prototypes from function prototype
------------------------------------------------------------------------
function luaU:DumpFunction(f, p, D)
  local source = f.source
  if source == p then source = nil end
  self:DumpString(source, D)
  self:DumpInt(f.lineDefined, D)
  self:DumpByte(f.nups, D)
  self:DumpByte(f.numparams, D)
  self:DumpByte(f.is_vararg, D)
  self:DumpByte(f.maxstacksize, D)
  self:DumpLines(f, D)
  self:DumpLocals(f, D)
  self:DumpUpvalues(f, D)
  self:DumpConstants(f, D)
  self:DumpCode(f, D)
end

------------------------------------------------------------------------
-- dump Lua header section (some sizes hard-coded)
------------------------------------------------------------------------
function luaU:DumpHeader(D)
  self:DumpLiteral(self.LUA_SIGNATURE, D)
  self:DumpByte(self.VERSION, D)
  self:DumpByte(self:endianness(), D)
  self:DumpByte(4, D)  -- sizeof(int)
  self:DumpByte(4, D)  -- sizeof(size_t)
  self:DumpByte(4, D)  -- sizeof(Instruction)
  self:DumpByte(luaP.SIZE_OP, D)
  self:DumpByte(luaP.SIZE_A, D)
  self:DumpByte(luaP.SIZE_B, D)
  self:DumpByte(luaP.SIZE_C, D)
  self:DumpByte(8, D)  -- sizeof(lua_Number)
  self:DumpNumber(self.TEST_NUMBER, D)
end

------------------------------------------------------------------------
-- dump function as precompiled chunk
-- * w, data are created from make_setS, make_setF
------------------------------------------------------------------------
function luaU:dump(L, Main, w, data)
  local D = {}  -- DumpState
  D.L = L
  D.write = w
  D.data = data
  self:DumpHeader(D)
  self:DumpFunction(Main, nil, D)
  -- added: for a chunk writer writing to a file, this final call with
  -- nil data is to indicate to the writer to close the file
  D.write(nil, D.data)
end

------------------------------------------------------------------------
-- find byte order (from lundump.c)
-- * hard-coded to little-endian
------------------------------------------------------------------------
function luaU:endianness()
  return 1
end

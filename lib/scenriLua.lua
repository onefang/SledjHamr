--[[  scenriLua - In world objects as skang Things.

TODO - Yes I know this breaks our design goals,
       but it's the easy way to quickly whip up in world object loading.
       Later a C + eet version will be written to fit our design,
       and this Lua based version will be kept.
       The system should "compile" Lua based files to eet as well.
       For now Lua based files is easier to debug.

]]

do
  local skang = require 'skang'
  -- This module has no default skin, it creates no windows.
  local _M = skang.moduleBegin('scenriLua', nil, 'Copyright 2014 David Seikel', '0.1', '2014-06-01 13:23:00')

  MeshType = {}
  TextureType = {}

  loadSim = function(sim)
    local file = sim .. '/index.omg'

    -- TODO - Stuffs should be local, but externally loaded functions can't access locals it seems.
    Stuffs = {}
    local simData, err = loadfile(file)

    if simData then
      setfenv(simData, getfenv(1))
      simData()
      if Stuffs.details and Stuffs.details.stuffs then
        local count = 0
        -- Lua is useless sometimes.
        for k, v in pairs(Stuffs.details.stuffs) do
          count = count + 1
        end
        preallocateStuffs(count)
        for k, v in pairs(Stuffs.details.stuffs) do
          partFillStuffs(k, v.fileName, v.pos[1], v.pos[2], v.pos[3], v.rot[1], v.rot[2], v.rot[3], v.rot[4])
        end
      end
    elseif 'cannot open ' ~= string.sub(err, 1, 12) then
      print("ERROR - " .. err)
    end
    print('Loaded sim from file ' .. file)
  end
  skang.thingasm('loadSim', 'Load a sim.', loadSim, 'string')

  finishLoadStuffs = function(sim, stuffs)
    local file = sim .. '/' .. stuffs
    local name = string.sub(stuffs, 1, -5)
    simData, err = loadfile(file)
    if simData then
      setfenv(simData, getfenv(1))
      simData()
      if Stuffs.details and Stuffs.details.Mesh then
        local meshFile = Stuffs.details.Mesh.fileName or stuffs
        local eStuffs = finishStuffs(Stuffs.uuid, Stuffs.name, name, Stuffs.description, Stuffs.owner, meshFile, MeshType[Stuffs.details.Mesh.kind], Stuffs.fake)
        if eStuffs then
          addMaterial(eStuffs, -1, TextureType[Stuffs.details.Mesh.materials[0].kind ], Stuffs.details.Mesh.materials[0].texture)
        end
      end
    elseif 'cannot open ' ~= string.sub(err, 1, 12) then
      print("ERROR - " .. err)
    end
  end
  skang.thingasm('finishLoadStuffs', 'Finish loading a stuffs.', finishLoadStuffs, 'string,string')

  loadStuffs = function(sim, stuffs)
    local file = sim .. '/' .. stuffs
    print('Rezzing ' .. file)
    simData, err = loadfile(file)
    if simData then
      setfenv(simData, getfenv(1))
      simData()
      if Stuffs.details and Stuffs.details.Mesh then
        local meshFile = Stuffs.details.Mesh.fileName or v.fileName
        local eStuffs = addStuffs(Stuffs.uuid, Stuffs.name, Stuffs.description,
          Stuffs.owner, v.fileName, MeshType[Stuffs.details.Mesh.kind],
          v.pos[1], v.pos[2], v.pos[3], v.rot[1], v.rot[2], v.rot[3], v.rot[4])
        if eStuffs then
          addMaterial(eStuffs, -1, TextureType[Stuffs.details.Mesh.materials[0].kind ], Stuffs.details.Mesh.materials[0].texture)
          stuffsSetup(eStuffs, Stuffs.fake)
        end
      end
    elseif 'cannot open ' ~= string.sub(err, 1, 12) then
      print("ERROR - " .. err)
    end
  end
  skang.thingasm('loadStuffs', 'Load a stuffs.', loadStuffs, 'string')

  skang.moduleEnd(_M)
end

--[[

4D early reflection calculation for heref~

--]]

-- utility 4D maths

local v4sub = function(u,v)
  local s = { }
  for i = 1,4 do s[i] = u[i]-v[i] end
  return s
end

local v4mul = function(u,k)
  local s = { }
  for i = 1,4 do s[i] = u[i]*k end
  return s
end

local v4dot = function(u,v)
  local d = 0
  for i = 1,4 do d = d + u[i]*v[i] end
  return d
end

local v4ref = function(x,p)
  local n = p.normal
  local d = p.distance
  return v4sub(x, v4mul(n, 2 * (v4dot(x,n) - d) / v4dot(n,n)))
end

local c = 340 / 1000

local v4time = function(x,y)
  local v = v4sub(x,y)
  local d = v4dot(v,v)
  return math.sqrt(d) / c
end

-- main class

local R = pd.Class:new():register("heref-calc")

function R:initialize(sel, atoms)
  self.inlets = 3
  self.outlets = 1
  return true
end

-- room size
function R:in_3_list(atoms)
  self.wall = {
    { normal = {  1, 0, 0, 0 }, distance = atoms[1] },
    { normal = { -1, 0, 0, 0 }, distance = 0 },
    { normal = { 0,  1, 0, 0 }, distance = atoms[2] },
    { normal = { 0, -1, 0, 0 }, distance = 0 },
    { normal = { 0, 0,  1, 0 }, distance = atoms[3] },
    { normal = { 0, 0, -1, 0 }, distance = 0 },
    { normal = { 0, 0, 0,  1 }, distance = atoms[4] },
    { normal = { 0, 0, 0, -1 }, distance = 0 }
  }
end

-- source position
function R:in_2_list(atoms)
  self.vsource = { atoms }
  for i = 1,8 do table.insert(self.vsource, v4ref(atoms, self.wall[i])) end
end

-- listener position
function R:in_1_list(atoms)
  local herefs = { }
  for i = 1,9 do table.insert(herefs, v4time(atoms, self.vsource[i])) end
  self:outlet(1, "list", herefs)
end

local R = pd.Class:new():register("hverb-calc")

function R:initialize(sel, atoms)
  if type(atoms[1]) ~= "number" then
    return false
  end
  self.count = math.max(atoms[1], 1)
  self.inlets = 1
  self.outlets = 1
  return true
end

-- note: gcd(0,0) == 0, which is what we want, if not math-correct
local gcd = function(m, n) 
  while m ~= 0 do m, n = math.mod(n, m), m end
  return n
end

-- precalculate fundamental modes
local fundamentals = { }
for nx = 0, 16 do
for ny = 0, 16 do
for nz = 0, 16 do
for nw = 0, 16 do
  if gcd(gcd(gcd(nx, ny), nz), nw) == 1 then
    table.insert(fundamentals, { nx, ny, nz, nw })
  end
end end end end

function R:delay(lx, ly, lz, lw, nx, ny, nz, nw)
  return 1000 / ((340/2) * math.sqrt((nx*nx)/(lx*lx)+(ny*ny)/(ly*ly)+(nz*nz)/(lz*lz)+(nw*nw)/(lw*lw)))
end

function R:in_1_list(atoms)
  local lx = atoms[1]
  local ly = atoms[2]
  local lz = atoms[3]
  local lw = atoms[4]
  local delays = { }
  for i,f in ipairs(fundamentals) do
    table.insert(delays, self:delay(lx, ly, lz, lw, unpack(f)))
  end
  table.sort(delays, function (a, b) return a > b end)
  local out = { }
  local j = 1
  for i = 1, self.count do
    out[i] = delays[j]
    repeat
      j = j + 1
    until delays[j] ~= delays[j-1]
  end
  self:outlet(1, "list", out)
end

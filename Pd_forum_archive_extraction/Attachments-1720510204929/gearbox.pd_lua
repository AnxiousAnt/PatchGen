local G = pd.Class:new():register("gearbox")

math.randomseed(os.time())

local function distance(p, q)
  local dx = p.x - q.x
  local dy = p.y - q.y
  return math.sqrt(dx * dx + dy * dy)
end

function G:initialize(sel, atoms)
  self.inlets = 1
  self.outlets = 2
  self.spawnhue = 0
  self.spawnparity = true
  self.spawnrate = 10
  self.spawncounter = 0
  self.scrollspeed = 0.01
  self.scrollangle = 0
  self.friction = 0.975
  self.dt = 0.002
  self.gears = { }
  return true
end

function G:spawn()
  self.spawnhue = math.fmod(self.spawnhue + 1 / 32, 1)
  local t = self.spawnhue * 2 * math.pi
  -- rec.709 yuv2rgb
  local r = 0.5 + 0.25 * 1.28033 * math.sin(t)
  local g = 0.5 - 0.25 * 0.21482 * math.cos(t) - 0.25 * 0.38059 * math.sin(t)
  local b = 0.5 + 0.25 * 2.12798 * math.cos(t)
  local p = math.random() > 0.5
  local c = math.floor(math.sqrt(math.random() * (144 - 25) + 25))
  local s = 3 * c / 24
  local h = (math.random() - 0.5) * 10
  local da = 0.4 * (math.random() - 0.5)
  local a
  if p then a = 2 * math.pi / c else a = - 2 * math.pi / c end
  local v = -0.5-math.random()
  table.insert(self.gears,
    { parity = p
    , cogs = c
    , size = s
    , mass = s * s
    , angle = math.random() * 2 * math.pi
    , dangle = 4 * a
    , position = { x = 20 * math.cos(self.scrollangle + da), y = 20 * math.sin(self.scrollangle + da) }
    , dposition = { x = v * math.cos(self.scrollangle + da), y =  v * math.sin(self.scrollangle + da) }
    , color = { r = r, g = g, b = b }
    , shape = 
        { cogs = c
        , spokes = math.random(2, 12)
        , dir = (math.random() - 0.5) * 8
        , sweep = math.random() * 8 + 1
        , teeth = 1 / c
        , hole = math.random() * 0.1 + 0.1
        , radiusin = math.random() * 0.1 + 0.3
        , radiusout = math.random() * 0.2 + 0.5
        , threshin = math.random() * 0.6 + 0.2
        , threshout = math.random() * 0.6 + 0.2
        }
    })
end

function G:cull()
  local gears = { }
  for i,g in ipairs(self.gears) do
    local r2 = g.position.x * g.position.x + g.position.y * g.position.y
    if r2 < 500 then
      local ok = true
      for j,h in ipairs(self.gears) do
        if g ~= h then
          if distance(g.position, h.position) < (g.size + h.size) / 2 then ok = false ; break end
        end
      end
      if ok then table.insert(gears, g) end
    end
  end
  self.gears = gears
end

function G:in_1_bang()
  if self.spawncounter == 0 then self:spawn() ; self.spawncounter = self.spawnrate else self.spawncounter = self.spawncounter - 1 end
  local g, h
  for z = 1,8 do
--    self.scrollangle = self.scrollangle + 0.02 * (math.random() - 0.5)
    for i,g in ipairs(self.gears) do
      local frx = 0
      local fry = 0
      for j, h in ipairs(self.gears) do
        if g ~= h then
          local d = distance(g.position, h.position)
          local e = d - (g.size + h.size)
          local k = 0
          if e > 0 then
            if g.parity == h.parity then k = - 1.3 * e else k = e end
          else
            k = 8 * e
          end
          local m = k * h.mass / (e * e)
          if math.abs(e) > 0.001 then
            frx = frx + math.tanh(m * (h.position.x - g.position.x) / 16) * 16
            fry = fry + math.tanh(m * (h.position.y - g.position.y) / 16) * 16
          end
        end
      end
      g.dposition.x = self.friction * g.dposition.x + frx * self.dt
      g.dposition.y = self.friction * g.dposition.y + fry * self.dt
    end
    for i,g in ipairs(self.gears) do
      g.angle = g.angle + self.dt * g.dangle
      g.position.x = g.position.x + g.dposition.x * self.dt - self.scrollspeed * math.cos(self.scrollangle)
      g.position.y = g.position.y + g.dposition.y * self.dt - self.scrollspeed * math.sin(self.scrollangle)
    end
  end
  self:cull()
  for i,g in ipairs(self.gears) do
    local s = g.shape
    local c = g.color
    local a = self.scrollangle
    self:outlet(2, "gear", { s.cogs, s.spokes, s.dir, s.sweep, s.teeth, s.hole, s.radiusin, s.radiusout, s.threshin, s.threshout, c.r, c.g, c.b, g.size, g.position.x, g.position.y, g.angle })
    self:outlet(1, "bang", { })
  end
end

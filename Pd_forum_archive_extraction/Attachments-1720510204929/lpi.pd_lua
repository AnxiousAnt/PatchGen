--[[ 
  lpi
  Output the value of pi
  author Martin Peach 20150129
--]]

-- Pd class

local lpi = pd.Class:new():register("lpi")
local digitstr = "%g" -- default format string gives up to 6 significant digits

function lpi:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 2
  return true
end

-- lpi:bang: output pi as float
function lpi:in_1_bang()

  local pi = math.atan(1.)*4.0
  pd.post(pi) 
  self:outlet(1, "float", {pi})
  self:outlet(2, "symbol", {string.format('%.99f', pi)}) -- 99 decimal places is maximum allowed but precision is less

end

-- end of lpi


--[[

-- Pd class written in Lua
-- fbar 2008

--]]

-- Change name of Pd class here
-- Use the same name as the filename (minus ".pd_lua")

local pdclassname = "ADD_CLASSNAME_HERE"

----------------------------------------
-- papa's little helpers
----------------------------------------
local function checkfloat(f)
    return type(f) == "number"
end

local function checksym(s)
    return type(s) == "string"
end
----------------------------------------


----------------------------------------
-- the actual class
----------------------------------------
local M = pd.Class:new():register(pdclassname)

function M:initialize(name, atoms)
    self.inlets = 1
    self.outlets = 1 
    
    --[[    load arg1 into self:
    if checkfloat(atoms[1]) then
        self.f = atoms[1]
    elseif checksym(atoms[1]) then
        self.s = atoms[1]
    end
    --]]
    return true
end

----------------------------------------
-- various methods 
--   uncomment them by replacing 
--   "--[[" with "---[[" (3 dashes)
----------------------------------------

--[[
function M:in_1_bang()
    self:outlet(1, "bang", {})
end
--]]


--[[
function M:in_1_float(f)
    self:outlet(1, "float", {f})
end
--]]


--[[
function M:in_1_error()
    self:error(pdclassname .. " crashed horribly.")
end
--]]


--[[
function M:in_1(sel, atoms)
    self:outlet(1, sel, atoms)
end
--]]


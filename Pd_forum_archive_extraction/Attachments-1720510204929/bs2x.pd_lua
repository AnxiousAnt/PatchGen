-- bs2x class: replace backslashes in symbol with something else (default: "/" or arg1)
-- fbar 2008

local M = pd.Class:new():register("bs2x")

function M:initialize(name, atoms)
    self.x = "/"
    if (type(atoms[1]) == "number") or (type(atoms[1]) == "string") then
        self.x = atoms[1]
    end
    self.inlets = 2
    self.outlets = 1 
    return true
end

function M:in_1_symbol(s)
    local result = string.gsub(s, "\\", self.x)
    self:outlet(1, "symbol", {result})
end

function M:in_2_symbol(s)
    self.x = s
end

function M:in_2_float(f)
    self.x = f
end


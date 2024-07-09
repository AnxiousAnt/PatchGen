-- Written by Frank Barknecht in 2007, use however you like.

local M = pd.Class:new():register("lstringpad")

function M:initialize(name, atoms)
    self.inlets = 2
    self.outlets = 1
    self.padlen = atoms[1] or 0
    self.pad = atoms[2] or ""
    return true
end


function M:in_1_symbol(s)
    local res = s .. string.rep(self.pad, self.padlen - s:len())
    self:outlet(1, "symbol", {res})
end

function M:in_2_float(f)
    self.padlen = f
end

function M:in_2_symbol(s)
    self.pad = s
end



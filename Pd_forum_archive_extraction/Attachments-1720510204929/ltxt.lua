-- ltxt: a simple hash type for Pd written in Lua.

local NAME_OF_CLASS = "ltxt" -- should be equal to the name of this file minus suffix

-- register this class with Pd as NAME_OF_CLASS
local M = pd.Class:new():register(NAME_OF_CLASS)

-- M now is used as the name for the class in this file.

function M:initialize(name, args)
    self.outlets = 2
    self.inlets = 1
    self.mydata = {} -- empty table to store whatever
    return true
end

-- lookup key in mydata, bang second outlet if not found:
function M:lookup(x)
    if self.mydata[x] then
        self:outlet(1, "list", self.mydata[x])
    else
        self:outlet(2, "bang", {})
    end
end

-- float and symbol methods will look up key in mydata table:
function M:in_1_float(x)
    self:lookup(x)
end

function M:in_1_symbol(x)
    self:lookup(x)
end

-- what to do on "add [atoms]" into 1st inlet:
function M:in_1_add(atoms)
    local key = table.remove(atoms, 1) -- pop first element of "atoms" into "key"
    self.mydata[key] = atoms
end

-- what to do on "print" into 1st inlet:
function M:in_1_print()
    for k,v in pairs(self.mydata) do
        pd.post(k .. ": " .. table.concat(self.mydata[k], " "))
    end
end

-- what to do on "reset" into 1st inlet:
function M:in_1_reset()
    self.mydata = {}
end

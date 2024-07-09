--[[ 

str2ms
    Tries to convert symbols to milliseconds
    MP 20120215 
    --Written by Martin Peach 
--]]

-- Pd class

local str2ms = pd.Class:new():register("str2ms")
local result -- save most recent result for bang

function str2ms:initialize(sel, atoms)
    self.inlets = 1
    self.outlets = 1
--pd.post("str2ms init")
    return true
end

function str2ms.str2ms(s)
    local values = {}
    local types = {}
    local multipliers = {s = 1000, m = 60000, h = 3600000, d = 86400000} --milliseconds

    for w in string.gmatch(s, "%d+") do
       table.insert(values, w)
    end
    for w in string.gmatch(s, "%a+") do
       table.insert(types, w)
    end
    if #values ~= #types then
        return "number/type mismatch"
    end
    for i = 1, #types do
        if (multipliers[types[i]]) == nil then
            return "unknown time type "..types[i]
        end
    end
    local totalms = 0

    for i = 1, #types do
        totalms = totalms + (multipliers[types[i]]*values[i])
    end

    return totalms 
end

function str2ms:in_1(sel, atoms)
    local ms = 0
    local str

--pd.post("str2ms:in1:"..sel..#atoms.." atoms")

    if sel == "float" then -- float is already in milliseconds
        result = atoms[1]
        self:outlet(1, "float", {result})
        return
    elseif sel == "symbol" then -- symbol is atom[1], may be convertible
        str = atoms[1]
    elseif sel == "pointer" then -- invalid type
        pd.post(self._name..": no method for pointer")
        return
    elseif sel == "bang" then -- maybe repeat previous output
        if type(result) == "number" then -- valid conversion
            self:outlet(1, "float", {result})
        end
        return
    elseif #atoms > 1 then -- lists are fraught
        pd.post(self._name..": no method for list")
        return
    elseif #atoms == 0 then -- selector may be convertible to milliseconds
        str = sel
    end
    result = self.str2ms(str) -- try to convert
    if type(result) == "number" then -- valid conversion
        self:outlet(1, "float", {result})
    elseif type(result) == "string" then -- error string
        pd.post(self._name..": "..result)
    end
end

--[[ 

    str2ms
    Tries to convert lists of floats like 
    91 91 91 50 54 54 49 44 50 56 54 93 93 44 91 91 49 51 54 53 44 52 49 56 93 93 93 13 10
    to four floats
    --{{{2705,246}},{{1358,402}}} square brackets

    MP 20120216 
    Written by Martin Peach 
--]]

-- Pd class

local bytes2pp = pd.Class:new():register("bytes2pp")
local result -- save most recent result for bang

function bytes2pp:initialize(sel, atoms)
    self.inlets = 1
    self.outlets = 4
--pd.post("bytes2pp init")
    return true
end

function bytes2pp:in_1(sel, atoms)
    local n = 0
    local str = ''

--pd.post("bytes2pp:in1:"..sel.." "..#atoms.." atoms")

    if sel == "list" then -- float is already in milliseconds
        for i = 1, #atoms  do
            if (type(atoms[i]) ~= "number") then
                pd.post(self._name..": "..atoms[i].." is not a float, it'a a "..type(atoms[i]))
                return            
            end
            str = str..string.char(atoms[i])
        end
--pd.post(str)
        i = 1
        for w in string.gmatch(str, "%d+") do
--pd.post(w.." is a "..type(w)) -- a string
            n = 0 + w -- force it to be a number
            self:outlet(i, "float", {n})
            i = i+1 -- next outlet
        end
    else
        pd.post(self._name..": no method for "..sel)
    end
end

--[[

note2num: convert note names like "C#" or "Bb" or "Ab#bb" to midi note
numbers

-- fbar 2008

--]]

local M = pd.Class:new():register("note2num")

local n2n = {
    c = 0,
    d = 2,
    e = 4,
    f = 5,
    g = 7,
    a = 9,
    b = 11,
}

function M:initialize(name, atoms)
    self.inlets = 1
    self.outlets = 1 
    return true
end

function M:in_1(sel, atoms)
    local s
    if not atoms[1] then
        s = sel
    else
        s = atoms[1]
    end
    assert(type(s) == "string", "only symbols allowed")
    s = string.lower(s)

    local n, mods = s:match("^([cdefgab])([#b]*)$")
    local val = n2n[n]
    
    if mods and val then
        local _, b = string.gsub(mods, "b", "b")
        local _, h = string.gsub(mods, "#", "#")
        val = val + h - b
    end

    if val then
        self:outlet(1, "float", {val})
    end
end


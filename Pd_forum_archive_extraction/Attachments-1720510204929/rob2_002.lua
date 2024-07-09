local NAME_OF_CLASS = "rob2"
local M = pd.Class:new():register(NAME_OF_CLASS)

function M:initialize(name, args)
    self.outlets = 2
    self.inlets = 1
    self.mydata = {} 
    return true
end

-- "insert [list]" into 1st inlet:
function M:in_1_insert(list)
    table.insert(self.mydata, list) 
end

-- what to do on "scores" into 1st inlet:
function M:in_1_scores()
for i, v in ipairs(self.mydata) do 
    if string.find(table.concat(v, " "), "score = 1", 1, true) then 
        self:outlet(1, "foo", v) end 
    end 
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

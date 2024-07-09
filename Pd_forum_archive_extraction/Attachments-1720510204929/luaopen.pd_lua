local pdclassname = "luaopen"

local M = pd.Class:new():register(pdclassname)

function M:initialize(name, atoms)
    self.inlets = 1
    self.outlets = 2 
    return true
end

function M:in_1_run(args)
    local cmd = table.concat(args, " ")
    -- instead of io.popen os.execute may be interesting as well.
    local result, err = io.popen(cmd)
    if result then
        for l in result:lines() do
            self:outlet(1, "list", {l})
        end
        result:close()
    else
        self:outlet(2, "error", {err})
    end
end



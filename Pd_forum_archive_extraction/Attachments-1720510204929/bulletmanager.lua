--------------------------------------------------------------------------
-- Use of coroutines to implement a simple scheduler (as in "Game Prog. 
-- Gems 6")
--------------------------------------------------------------------------

local manager = {
    tasks = {}
}

function manager:addtask (name, task)
    self.tasks[name] = coroutine.create(task)
end

function manager:step ()
    for name,co in pairs(self.tasks) do
        coroutine.resume(co)
        if coroutine.status(co) == "dead" then
            self.tasks[name] = nil
        end
    end
end

--------------------------------------------------------------------------
-- Bullet coroutine: This is the task that will get created for every bullet 
-- and resumed until dead.
--------------------------------------------------------------------------

local function bullet(startangle, name, printobj)
    startangle = startangle or 0
    name = name or "unknown"
    printobj = printobj or error("supply a printer object", 2)
    local co = function(scheduler)
        local age = 0
        -- inital pos and rot:
        local x = 0
        local y = 0
        local angle = math.rad(startangle)
        -- motion:
        local step = 0.3
        local rot = math.rad(11)

        while (-4 < x) and (x < 4) and (-4 < y) and (y < 4) and (age < 100) do
            printobj:printfun({name, x, y, age})
            coroutine.yield()
            angle = angle + rot
            x = x + step * math.cos(angle)
            y = y + step * math.sin(angle)
            age = age + 1
        end
    end
    return co
end

--------------------------------------------------------------------------
-- Pd class
--------------------------------------------------------------------------

local BulletManager = pd.Class:new():register("bulletmanager")

function BulletManager:initialize(name)
    self.outlets = 1
    self.inlets = 1
    self.b = {}
    self.s = manager
    return true
end

function BulletManager:in_1_bang()
    self.s:step()
end

function BulletManager:in_1_add(atoms)
    if type(atoms[1]) == "number" then
        local b = atoms[1]
        self.s:addtask(b, bullet(b*5, b, self))
    else
        self:error("wrong type: add <float>")
    end
end

function BulletManager:printfun(msg)
    self:outlet(1, "list", msg)
end


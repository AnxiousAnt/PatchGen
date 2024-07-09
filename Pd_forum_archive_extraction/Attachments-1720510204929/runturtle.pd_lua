require 'smtut'

local M = pd.Class:new():register("runturtle")

function M:initialize(name, atoms)
    self.inlets = 1
    self.outlets = 1

    -- painter function for turtles:
    local painter = function (x,y,oldx,oldy, dir) 
        self:paint({x,y,oldx,oldy, dir}) 
    end
    
    self.t = Turtle(painter)
    return true
end

function M:in_1(action, atoms)
    self.t(action, atoms)
end

function M:paint(coords)
    self:outlet(1,"list", coords)
end

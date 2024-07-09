-- -----------------------------
-- test code marius 02/08
-- marius.schebella@gmail.com
-- -----------------------------

require 'luagl'

local msgltest2 = pd.Class:new():register("msgltest2")

function msgltest2:initialize(name, atoms)
    self.inlets = 3
    self.max = 3
    self.drawmode = 3
    pd.post(tostring(self.drawmode))
    pd.post(tostring(self))
    return true
end

function msgltest2:in_1(sel, atoms)
    if sel == "gem_state" then
       msgltest2:render(self)
    end
end

function msgltest2:in_2_float(f)
    self.max = math.abs(f)
    pd.post(self.max)
end

function msgltest2:in_3_float(g)
	self.drawmode = math.abs(g)
end

function msgltest2:render(myself)
    max = math.max(myself.max, 1)
    glBegin(myself.drawmode)
    glColor3d(math.random(), math.random(), math.random())
    for i=1,max do
       glVertex2d(math.random(-400,400)/100, math.random(-400,400)/100)
    end
    glEnd()
end

require 'luagl' 

local M = pd.Class:new():register("linebuffer")

function M:initialize(name, atoms)
    self.inlets = 2
    self.outlets = 1
    self.buffer = {}
    self.aging = 0.01
    self.smooth = false
    return true
end

function M:in_1_reset()
    self.buffer = {}
end

function M:in_2_list(atoms)
    for i=1,4 do
        assert(type(atoms[i]) == "number", "Error: All 4 coordinates must be numbers!")
    end
    atoms.age = 1
    self.buffer[#self.buffer + 1] = atoms
end

function M:in_1_aging(atoms)
    self.aging = math.max(0, atoms[1]) or 0.01
end

function M:in_1_smooth(atoms)
    if atoms[1] and atoms[1] ~= 0 then
        self.smooth = true
    else
        self.smooth = false
    end
    if self.smooth then
        glEnable(GL_LINE_SMOOTH)
    else
        glDisable(GL_LINE_SMOOTH)
    end
end


function M:render()
    for i,l in ipairs(self.buffer) do
        c = l.age
        glBegin(GL_LINES)
            glColor4d(1,1,1,c)
            glVertex2d(l[1], l[2])
            glVertex2d(l[3], l[4])
        glEnd()
        l.age = l.age - self.aging 
    end
    for i,l in ipairs(self.buffer) do
        if l.age <= 0 then table.remove(self.buffer, i) end
    end

end

function M:in_1_gem_state()
    self:render(self)
    self:outlet(1, "float", {#self.buffer})
end

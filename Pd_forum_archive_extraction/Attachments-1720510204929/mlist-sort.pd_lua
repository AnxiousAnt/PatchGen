local C = pd.Class:new():register("mlist-sort")

function C:initialize(name, atoms)
  self.inlets = 2
  self.outlets = 1
  self.lists = { }
  return true
end

function C:in_2(sel, atoms)
  table.insert(self.lists, { sel = sel, atoms = { unpack(atoms) } })
end

function C:in_1_float(f)
  local index = math.floor(f)
  local sorter
  if index <= 0 then
    sorter = function(a,b) return a.sel < b.sel end
  else
    sorter = function(a,b) return a.atoms[index] < b.atoms[index] end
  end
  table.sort(self.lists, sorter)
  for k,v in ipairs(self.lists) do
    self:outlet(1, v.sel, v.atoms)
  end
end

function C:in_1_flush()
  self.lists = { }
end

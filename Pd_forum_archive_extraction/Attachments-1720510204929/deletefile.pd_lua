--[[ 

--deletefile
--  Symbol input is name of file to delete
-- Martin Peach 20130127
--]]

-- Pd class

local deletefile = pd.Class:new():register("deletefile")

function deletefile:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  return true
end

function deletefile:in_1_delete(atom)
  pd.post(type(atom[1]))
  if type(atom[1])=="string" then
    --self:outlet(1, "symbol", {atom[1]})
    f = io.open(atom[1], "r")
    if (f == nil) then
      pd.post("can't open " .. atom[1])
    else
      pd.post("deleting " .. atom[1])
      result = os.remove(atom[1])
      if (result) then q = 1
      else
        q = 0
      end
      self:outlet(1, "float", {q})
    end
  else
    pd.post("need a file name")
  end
end

#! /usr/bin/env lua
-- A script to print out all the comments from each Pd help file found under
-- the path passed as the first argument, defaults to the directory from which the script is run.
-- Author Martin Peach 20121011

require"lfs"
path = "." -- default search path
totalFiles = 0

function attrdir (path)
  local file ourPdHelpFile = nil

--  print ("Searching " .. path)
  for file in lfs.dir(path) do
    if file ~= "." and file ~= ".." then
      local f
--      if path:find("/", -1) then f = path..file
--      else f = path..'/'..file end
      while path:find("/", -1) do path = path:sub(1,-2) end -- strip any trailing slash(es) from path
      f = path..'/'..file
      --print ("\t "..f)
      local attr = lfs.attributes (f)
      if (type(attr) ~= "table") then break end
      assert (type(attr) == "table", f)
      if attr.mode == "directory" then
        attrdir (f)
      else
        local ss, s, e, x, y
        local objName

        s, e = file:find("-help.pd")
        if e == file:len() then -- a filename ending in "-help.pd"
          totalFiles = totalFiles + 1
          objName = file:sub(1,s-1)
          print (totalFiles .. ": [" .. objName .. "]" .. " at " .. f)
          ourPdHelpFile = io.open(f)
          if ourPdHelpFile == nil then
            print ("Can't open " .. f)
          else
-- an example comment from a pd patcher:
--  #X text 86 306 This example measure the time it takes for two different
--  methods to count from 0 to 999;
            local canvasDepth = 0
            --local maxID = 0
            local canvasName = {}
            local canvasFrom = {}
            local canvasID = 0
            local canvasNow = 0
            local commentLines = {}
            local sortedLines = {}

            repeat
              ourLine = ourPdHelpFile:read()
              if ourLine ~= nil then
                if ourLine:find("^#N canvas") then -- line starts with "#N canvas"
                  canvasDepth = canvasDepth + 1
                  canvasID = canvasID + 1
                  canvasFrom[canvasID] = canvasNow
                  canvasNow = canvasID 
                  s, e = ourLine:find("%a", 10) -- first text after "canvas" will be canvas name
                  if s then
                    ss, e = ourLine:find(" ", e) -- first space after canvas name
                   if not ss then
                      ourLine = ourLine:sub(s) -- name extends to end of line
                    else
                      ourLine = ourLine:sub(s, ss-1) -- name followed by number
                    end
                    canvasName[canvasID] = ourLine
                  else
                    canvasName[canvasID] = " "
                  end
                  --if canvasID > maxID then maxID = canvasID end
--print("a->canvasDepth " .. canvasDepth .. " canvasID " .. canvasID .. " canvasFrom " .. canvasFrom[canvasID] .. " canvasNow " .. canvasNow .. " maxID " .. maxID)
                elseif ourLine:find("^#X restore") then
                  canvasDepth = canvasDepth - 1
                  canvasNow = canvasFrom[canvasNow]
--print("b->canvasDepth " .. canvasDepth .. " canvasID " .. canvasID .. " canvasFrom " .. canvasFrom[canvasID] .. " canvasNow " .. canvasNow .. " maxID " .. maxID)
                else
--print("c->canvasDepth " .. canvasDepth .. " canvasID " .. canvasID .. " canvasFrom " .. canvasFrom[canvasID] .. " canvasNow " .. canvasNow .. " maxID " .. maxID)
                  s, e = ourLine:find("^#X text")
-- Comment begins with #X text:
                  if s == 1 then
                    ourLine = ourLine:sub(e+1)
-- First number is horizontal position of top left corner of comment box:
                    s, e = ourLine:find("%d+") 
                    x = ourLine:sub(s,e)
                    ourLine = ourLine:sub(e+1)
-- First number is vertical position of top left corner of comment box:
                    s, e = ourLine:find("%d+") 
                    y = ourLine:sub(s,e)
-- Remaining line is comment text:
                    ourLine = ourLine:sub(e+1)
                    ourLine = canvasNow .. ourLine
-- End of comment must be semicolon:
                    if not ourLine:find(";") then
                      repeat
                        moreLine = ourPdHelpFile:read()
                        if moreLine then ourLine = ourLine .. "\n    " .. moreLine end
                      until ((moreLine == nil) or (moreLine:find(";")))   
                    end -- if not ourLine:find(";$") then
-- prepare to sort the lines in order of y (They are usually not in order in the patcher file)
                    ourLine = ourLine:gsub(" \\,", ",") --  de-escape commas (look for space:backslash:comma)
                    ourLine = ourLine:gsub(" \\;", ";") --  de-escape semicolons
                    ourLine = ourLine:gsub("\\", "") --  remove any remaining backslashes
                    ourLine = ourLine:sub(1, -2) -- strip trailing semicolon 
                    commentLines[y] = ourLine -- store comments using their y position as key
                    --print ("<" .. x .. "> <" .. y .. "> ".. ourLine)
                  end -- if s == 1 then
                end -- if ourLine:find("^#N canvas") then
              end -- if ourLine ~= nil then

            until ourLine == nil
            --for x,line in pairs(commentLines) do
            --  print ("> " .. x .. " " ..line)
            --end
            --print("maxID " .. maxID)

-- now we sort the lines in order of y
            local i = 1
            for y, line in pairsByKeys(commentLines, function(a,b) return tonumber(a)<tonumber(b) end) do
              --print("*" .. y .. " " .. line)
              sortedLines[i] = line
              i = i+1
            end
-- now we print them out in order of canvasID
            --for patchID = 1, maxID do
            for patchID = 1, canvasID do
              print ("  {" .. patchID .. ": " .. canvasName[patchID] .. "}")
              for i = 1, #sortedLines do
                line = sortedLines[i]
                --print("*" .. line)
                s, e = line:find("^%d+") -- retrieve patchID from the start of the comment string
                if s == 1 then -- line begins with a number string
                  local n = tonumber(line:sub(s,e)) -- , which should be the patchID
                  if n == patchID then print("   " .. line:sub(e+1)) end -- if it's this patchID strip ID and print it
                end
              end
            end
            print (" ")
--            print ("  " .. canvasID-1 .. " subpatches in [" .. objName .. "]\n")
            commentLines = {}
          end -- if ourPdHelpFile == nil then
        end -- if e == file:len() then
      end -- if attr.mode == "directory" then
    end -- if file ~= "." and file ~= ".." then
  end --   for file in lfs.dir(path) do
  --print (" ")
end

function pairsByKeys (t, f)
  local a = {}
  for n in pairs(t) do
    table.insert(a, n) -- put the keys in table a
  end
  table.sort(a, f) -- sort the keys using f()
  local i = 0 -- iterator variable
  local iter = function () -- iterator function
    i = i + 1 -- step through a in sorted order
    if a[i] == nil then
      return nil
    else
      return a[i], t[a[i]] -- return the key and its value from t
    end
  end
  return iter
end

if arg[1] then path = arg[1] end
print ("Comments from all Pure Data help patchers found under " .. path .. ":")
attrdir(path)
print ("End of listing for " .. path .. ".")
print ("Total help files found: " .. totalFiles .. ".")


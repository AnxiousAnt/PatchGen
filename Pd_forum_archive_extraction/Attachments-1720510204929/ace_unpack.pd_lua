--[[ 
ace_unpack by Martin Peach based on
list-unpack
    Like [unpack] for any kind of type. Argument specifies number of outlets.
    pointers untested rsp. not supported.
    --Written by Frank Barknecht 
    --Fixed for pdlua-0.2svn by Clahde Heiland-Allen
-- ACE 1m mag text file looks like this:
--:Data_list: ace_mag_1m.txt
--:Created: 2012 Jun 04 2110 UT
--# Prepared by the U.S. Dept. of Commerce, NOAA, Space Weather Prediction Center
--# Please send comments and suggestions to SWPC.Webmaster@noaa.gov 
--# 
--# Magnetometer values are in GSM coordinates.
--# 
--# Units: Bx, By, Bz, Bt in nT
--# Units: Latitude  degrees +/-  90.0
--# Units: Longitude degrees 0.0 - 360.0
--# Status(S): 0 = nominal data, 1 to 8 = bad data record, 9 = no data
--# Missing data values: -999.9
--# Source: ACE Satellite - Magnetometer
--#
--#              1-minute averaged Real-time Interplanetary Magnetic Field Values 
--# 
--#                 Modified Seconds
--# UT Date   Time  Julian   of the   ----------------  GSM Coordinates ---------------
--# YR MO DA  HHMM    Day      Day    S     Bx      By      Bz      Bt     Lat.   Long.
--#------------------------------------------------------------------------------------
--2012 06 04  1910   56082   69000    0     4.9     2.1    -3.5     6.4   -33.6    22.8
--2012 06 04  1911   56082   69060    0     3.8     2.5    -4.5     6.4   -44.8    33.8

--]]

-- Some footils --

-- outlet selectors
local selectors = {"float", "list", "symbol", "bang", "pointer"}

-- check if item x is in table t:
local function contains(t, x)
  for _,v in ipairs(t) do
    if v == x then return true end
  end
  return false
end

-- Pd class

local AceUnpack = pd.Class:new():register("ace_unpack")
local inStr = ""
local lineCount = 0
local verbosity = 1

function AceUnpack:initialize(name, atoms)
    self.inlets = 1 -- inlet for a list of ascii characters
--    if atoms[1] == nil then 
    self.outlets = 15
    return true
--    elseif type(atoms[1]) == "number" and atoms[1] >= 0 then
--        self.outlets = math.max(atoms[1], 1)
--        return true
--    else
--        pd.post("ace-unpack: First arg must be a positive float or empty")
--        return false
--    end
end

function AceUnpack:Outlet(num, atom)
    -- a better outlet: automatically selects selector
    -- map lua types to pd selectors
    local selectormap = {string = "symbol", number="float", userdata="pointer"}
    self:outlet(num, selectormap[type(atom)], {atom})
end

function AceUnpack:in_1_reset()
    lineCount = 0
end

function AceUnpack:in_1_verbosity(atoms)
    pd.post (type(atoms[1]))
    if type(atoms[1] == number) then
        if verbosity > 0 then 
            pd.post("ace_unpack verbosity will be" .. atoms[1])
        end
        verbosity = atoms[1]
    else
        pd.post("ace_unpack wants a number for verbosity")
    end
    if verbosity > 0 then
        pd.post("ace_unpack verbosity is " .. verbosity)
    end
end

function AceUnpack:in_1(sel, atoms)
    for i=1, #atoms do
        inStr = inStr..string.char(atoms[i])
        if atoms[i] == 10 then -- end of line
            if inStr:sub(1, 1) == "2" then -- a data line starting with year number
            --if inStr:find(, "2") == 1 then -- a data line starting with year number
                --pd.post("valid")
                lineCount = lineCount + 1 -- first line is 1
                ix = inStr:find(" ") -- find the space after the number
                yr = inStr:sub(1, ix) -- year substring is between 1 and ix
                --pd.post(yr)
                inStr = inStr:sub(ix) -- strip the number from the beginning of the input string
                --pd.post (inStr)

                ix = inStr:find("[0-9]") -- find start of next number
                inStr = inStr:sub(ix) -- strip leading whitespace
                ix = inStr:find(" ") -- find start of trailing whitespace
                mo = inStr:sub(1, ix) -- month substring
                --pd.post(mo)
                inStr = inStr:sub(ix) -- strip the number from the beginning of the input string
                --pd.post (inStr)

                ix = inStr:find("[0-9]") -- find start of next number
                inStr = inStr:sub(ix) -- strip leading whitespace
                ix = inStr:find(" ") -- find start of trailing whitespace
                da = inStr:sub(1, ix) -- day substring
                --pd.post(da)
                inStr = inStr:sub(ix) -- strip the number from the beginning of the input string
                --pd.post (inStr)
 
                ix = inStr:find("[0-9]") -- find start of next number
                inStr = inStr:sub(ix) -- strip leading whitespace
                hh = inStr:sub( 1, 2) -- we split the HHMM string into HH and MM
                --pd.post(hh)
                inStr = inStr:sub( 3) -- strip the number from the beginning of the input string
                --pd.post (inStr)

                mm = inStr:sub( 1,2) -- we split the HHMM string into HH and MM
                --pd.post(mm)
                ix = inStr:find( " ") -- find start of trailing whitespace 
                inStr = inStr:sub( ix) -- strip leading number
                ix = inStr:find( "[0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip trailing whitespace
                --pd.post (inStr)

                ix = inStr:find( " ") -- find trailing whitespace
                jd = inStr:sub( 1, ix) -- jd substring
                --pd.post(jd)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                sx = inStr:sub( 1, ix) --seconds substring
                --pd.post(sx)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                s = inStr:sub( 1, ix) -- status substring
                --pd.post(s)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                Bx = inStr:sub( 1, ix) -- Bx substring
                --pd.post(Bx)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                By = inStr:sub( 1, ix) -- By substring
                --pd.post(By)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                Bz = inStr:sub( 1, ix) -- Bz substring
                --pd.post(Bz)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                Bt = inStr:sub( 1, ix) -- Bt substring
                --pd.post(Bt)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                Lat = inStr:sub( 1, ix) -- Lat substring
                --pd.post(Lat)

                --ix = inStr:find( " ") -- unnecessary
                inStr = inStr:sub( ix) -- strip the number from the beginning of the input string
                ix = inStr:find( "[-0-9]") -- find start of next number
                inStr = inStr:sub( ix) -- strip leading whitespace
                --pd.post (inStr)
                ix = inStr:find( " ") -- find trailing whitespace
                Lon = inStr:sub( 1, ix) -- Lon substring
                --pd.post(Lon)
                self:outlet(15, "float", {lineCount})
                self:outlet(14, "float", {tonumber(Lon)})
                self:outlet(13, "float", {tonumber(Lat)})
                self:outlet(12, "float", {tonumber(Bt)})
                self:outlet(11, "float", {tonumber(Bz)})
                self:outlet(10, "float", {tonumber(By)})
                self:outlet(9, "float", {tonumber(Bx)})
                self:outlet(8, "float", {tonumber(s)})
                self:outlet(7, "float", {tonumber(sx)})
                self:outlet(6, "float", {tonumber(jd)})
                self:outlet(5, "float", {tonumber(mm)})
                self:outlet(4, "float", {tonumber(hh)})
                self:outlet(3, "float", {tonumber(da)})
                self:outlet(2, "float", {tonumber(mo)})
                self:outlet(1, "float", {tonumber(yr)})
            else
                if (verbosity >  0) then 
                    pd.post (inStr)
                end
            end
            inStr = ""
        end
    end
end

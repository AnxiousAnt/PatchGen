-- single method turtle

function Turtle(painter)
    local x, y, oldx, oldy, dir, draw = 0, 0, 0, 0, 0, true
    local painter = painter or print -- drawing function

    return function (action, args)
        
        if action == "reset" then
            x, y, oldx, oldy, dir, draw = 0, 0, 0, 0, 0, true

        elseif action == "setpos" then
            x, y = args[1], args[2]
        
        elseif action == "setdir" then
            dir = args[1]
            dir = dir % 360
        
        elseif action == "turn" then
            dir = (dir + args[1]) % 360
        
        elseif action == "pen" then
            if args[1] and args[1] ~= 0 then
                draw = true
            else
                draw =  false
            end
        
        elseif action == "draw?" then
            return draw
        
        elseif action == "print" then
            print('x', x) 
            print('y', y)
            print('oldx', oldx)
            print('oldy', oldy) 
            print('dir', dir)
            print('draw', draw)
        
        elseif action == "forward" then
            local dist = args[1]
            oldx = x
            oldy = y
            x = x + dist * math.cos(math.rad(dir))
            y = y + dist * math.sin(math.rad(dir))
            if draw and type(painter) == "function" then
                painter(x, y, oldx, oldy, dir)
            end

        elseif action == "getpos" then
            return x, y
        
        elseif action == "getdir" then
            return dir
        
        else
            print("error: Unknown action: ", action)
        end
    end
end

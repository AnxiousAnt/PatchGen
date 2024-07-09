from PIL import Image
import math
import random 
i = Image.open("./small.png")

# some vars
rndx = 1
rndy = 1
spacex = 20
spacey = 20
guisize = 20

#use can be: tgl, bng, number or all
use = "all"

f = open('patch.pd', 'w')


f.write("#N canvas 0 53 1440 792 10;\n")
j = 0;
width, height = i.size
for x in range(width):
    for y in range(height):
		
		p = i.getpixel((x, y))
		
		if(p == 255): #white
			fgcolor = str(-262144)
		elif(p == 0 or p == 1 or p == 2 or p == 3): #black
			fgcolor = str(-1)
		else:
			fgcolor = str((((p+1) / 4 * 64 * 64) * -1) - ((p+1) / 4 * 64) - (p + 1) / 4)
		
		if(use == "all"):
			r = random.randint(0,1)
			if(r == 0):
				if(fgcolor != "-262144"):
					f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" bng "+str(guisize)+" 250 50 0 empty pn"+str(j)+" empty 17 7 0 10 "+fgcolor+" -1 -1;\n")
					
			elif(r == 1):
				if(fgcolor != "-262144"):
					f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" tgl "+str(guisize)+" 1 empty pn"+str(j)+" empty 17 7 0 10 "+fgcolor+" -1 -1 1 1;\n")
			elif(r == 2):
				f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" nbx 1 "+str(guisize)+" -1e+37 1e+37 0 0 empty pn"+str(j)+" empty 0 -8 0 10 "+fgcolor+" -1 -1 0 256;\n")
		elif(use == "number"):
				f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" nbx 1 "+str(guisize)+" -1e+37 1e+37 0 0 empty pn"+str(j)+" empty 0 -8 0 10 "+fgcolor+" -1 -1 0 256;\n")
		elif(use == "tgl"):
				f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" tgl "+str(guisize)+" 1 empty pn"+str(j)+" empty 17 7 0 10 "+fgcolor+" -1 -1 1 1;\n")
		elif(use == "bng"):
				f.write("#X obj "+str(x * spacex + random.randint(0,rndx))+" "+str(y * spacey + random.randint(1,rndy))+" bng "+str(guisize)+" 250 50 0 empty pn"+str(j)+" empty 17 7 0 10 "+fgcolor+" -1 -1;\n")
		
		j = j + 1

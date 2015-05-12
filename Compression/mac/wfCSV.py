import random

f = open("img.csv","w")

for x in xrange(10000):
    for y in xrange(10000):
        
        #pix = ev_tpc.at(0)[0]
        #print "Pixel: {0}".format(pix)
        r = random.random()
        f.write(str(128*((x/3200.)+r))+",")
        
    f.write("\n")
    
f.close()

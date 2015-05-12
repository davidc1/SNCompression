import ROOT, sys, os
from ROOT import *
from matplotlib import pyplot as plt
import numpy as np
import scipy
from PIL import Image

gSystem.Load('libAnalysis')
gSystem.Load('libLArUtil')
gSystem.Load('libSNCompression')

def colorgrad(x):
    
    f = 255/5.
    
    # assumes x in 0-255 range
    rgb = (0,0,0)
    #if (x < f):
    #    rgb = (int(255-x),0,255)
    if (x < f):
        rgb = (0,int(x),255)
    elif (x < 2*f):
        rgb = (0,255,int(255-x))
    elif (x < 3*f):
        rgb = (int(x),255,0)
    elif (x < 4*f):
        rgb = (255,int(255-x),0)
    else:
        rgb = (255,0,int(x))

    return rgb

filename = sys.argv[1]

geom = larutil.Geometry.GetME()

#open root file and go to scanner directory
f = ROOT.TFile(filename,"READ")
d = f.Get("scanner")

tpcTree = d.Get("tpcfifo_tree")
ev_tpc = larlight.tpcfifo()


#for event in tpcTree:

tpcTree.GetEvent(0)
    
# multiplicity
m = 2
fr = 1

ev_tpc = tpcTree.tpcfifo_branch

impil = Image.new( 'RGB', (fr*m*3400,m*3200), "blue" )
pixels = impil.load()

maxtick = 0

#image = np.zeros((3200*m,3000*m))
#image.fill(400)
counter = 0
try:
    counter = input('Hit Enter to continue to next evt, or type in an event number to jump to that event:')
except SyntaxError:
    counter = counter + 1
for w in ev_tpc:
    if (w.plane() == 2):
        chan_num = w.channel_number()
        wirenum = geom.ChannelToWire(chan_num)
        if (wirenum < 3400):
            print "Wire: {0}".format(wirenum)
            #time = w.readout_sample_number_RAW()+1
            #for n in xrange(m):
            for n in xrange(m):
                for tick in xrange(3200):
                    #image[tick,wirenum] = int((w[tick]))#-2040.
                    t = 0
                    if (w[tick]-398 > 0):
                        if (w[tick]-398>maxtick):
                            maxtick=w[tick]-398
                        aa = float((np.log(w[tick]-398.)))
                        #aa = float( (w[tick]-398.)/220. )
                        t = int(255*aa)
                        if (t > 255):
                            t = 255
                        for h in xrange(m*fr):
                            pixels[int(fr*m*wirenum+h),int(m*tick+n)] = colorgrad(t)

print maxtick
#image = (image-image.min())/(np.ptp(image))
#print image.min()
impil.save("Img/testV.png",quality=100)
#res = Image.fromarray(np.uint8(plt.cm.jet(image)*255))
#res.save("Img/AAA.png",quality=100)
#scipy.misc.imsave('out.jpg',image)
#for x in xrange(1000):
#        for y in xrange(1000):
#            if (image[x,y] != 0):
#                print image[x,y]
#implot = plt.imshow(image)
#implot 
#plt.colorbar(orientation='vertical')
#plt.show()
#print "hello"
#print w.plane()
#tpcTree.SetBranchAddress("tpcfifo_branch",ev_tpc)
#tpcTree.GetEntry(1)
#print ev_tpc[0].plane()

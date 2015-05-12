import ROOT, sys, os
from ROOT import *
from matplotlib import pyplot as plt
import numpy as np
from PIL import Image

gSystem.Load('libAnalysis')
gSystem.Load('libLArUtil')
gSystem.Load('libSNCompression')

filename = sys.argv[1]

#open root file and go to scanner directory
f = ROOT.TFile(filename,"READ")
d = f.Get("scanner")

tpcTree = d.Get("tpcfifo_tree")
ev_tpc = larlight.tpcfifo()
for event in tpcTree:
    
    counter = 0
    ev_tpc = tpcTree.tpcfifo_branch

    try:
        counter = input('Hit Enter to continue to next evt, or type in an event number to jump to that event:')
    except SyntaxError:
        counter = counter + 1

    # specify size of each image (in pixels)
    n_pixels = 512


    counter = 0

    #time-ticks (Y coordinate size)
    Ypixels = 3200*4 #each data point mapped to 5x5 pixel for better viewing quality
    Xpixels = 3200*4 # same here

    # so now figure out how many sub-images (n_pixels X n_pixels) there are
    nImagesX = int(float(Xpixels)/float(n_pixels)) + 1 
    nImagesY = int(float(Ypixels)/float(n_pixels)) + 1

    image = np.zeros((n_pixels,n_pixels))

    for ImgX in xrange(nImagesX):
        for ImgY in xrange(nImagesY):
            

            image = np.zeros((n_pixels,n_pixels))
            
            for x in xrange(n_pixels):
                for y in xrange(n_pixels):
                    
                    if ((ev_tpc.at(4796+int((ImgX*n_pixels+x)/4))[int((ImgY*n_pixels+y)/4)]-400) > 0):
                        image[x,y] = np.log(ev_tpc.at(4796+int((ImgX*n_pixels+x)/4))[int((ImgY*n_pixels+y)/4)]-400)

                        
            image = image/5.
            res = Image.fromarray(np.uint8(plt.cm.jet(image)*255))                
            res.save("Img/{0}_{1}.jpeg".format(ImgX,ImgY),quality=100)
            print "Saving Image [{0},{1}]".format(ImgX,ImgY)

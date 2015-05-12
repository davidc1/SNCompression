import ROOT, sys, os
from ROOT import *
from matplotlib import pyplot as plt
import numpy as np
from PIL import Image

gSystem.Load('libAnalysis')
gSystem.Load('libLArUtil')
#gSystem.Load('libSNCompression')

filename = sys.argv[1]

#open root file and go to scanner directory
f = ROOT.TFile(filename,"READ")
d = f.Get("scanner")

tpcTree = d.Get("tpcfifo_tree")
ev_tpc = larlight.tpcfifo()
for event in tpcTree:
    
    ev_tpc = tpcTree.tpcfifo_branch
    
    image = np.zeros((9600,3458))
    #image = np.zeros((800*5,800*5))
    #image.fill(400)
    counter = 0

    f = open("img.csv","w")


    for w in ev_tpc:
        if (w.plane() == 2):
            time = w.readout_sample_number_RAW()+1
            for tick in xrange(9600):
                #print w[tick]-390
                #intvar = (w[tick]-390)%200
                #print intvar
                #hexvar = hex(int((w[tick]-400)/200.))
                #hexnum = hexvar.split('x')[1]
                #hexfill = hex(2147483648+256*intvar+65536*intvar+intvar)
                #print hexfill
                #if (w.channel_number()-4796 < 800):
                if (w[tick]>400):
                    #                        for x in xrange(5):
                    #                            for y in xrange(5):
                    image[tick,(w.channel_number()-4796)] = w[tick]-400
                    f.write( str(128*(w[tick]-400.)/200)+",")

            f.write("\n")
            
    f.close()
    break
            
                #print "Tick: {0}".format(w[tick])
    #print "peak to peak: {0}".format(np.ptp(image))

    #pilimage = Image.frombuffer('RGBA',(3200,3458),image,'raw','RGBA',0,1)
    #pilimage.save('heximage.jpeg',quality=100)
    #image = (image-image.min())/(np.ptp(image))

    #implot = plt.imshow(image)
    #implot.set_cmap('spectral')
    #implot.set_clim(0.5,1.0)
    #plt.show()

    #plt.imshow(image)
    #plt.savefig("out.jpeg",dpi=200)
    #res = Image.fromarray((image*255).astype(np.uint8))
    #res = Image.fromarray(np.uint8(plt.cm.jet(image)*255))
    #print image.min()
    #scipy.misc.imsave("img.jpeg",image)
    #im = Image.fromarray(image)
    #res.save("fff.jpeg",quality=100)
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

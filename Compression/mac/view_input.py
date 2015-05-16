# Load libraries
import sys, os
from ROOT import *
from ROOT import gSystem
from ROOT import larlite as fmwk
from ROOT import compress
import matplotlib.pyplot as plt
import numpy as np

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
#my_proc.set_io_mode(fmwk.storage_manager.kBOTH)
#my_proc.set_io_mode(fmwk.storage_manager.kWRITE)
my_proc.set_io_mode(fmwk.storage_manager.kREAD)


for x in xrange(len(sys.argv)-2):
    my_proc.add_input_file(sys.argv[x+1])

#set output data file
#my_proc.set_output_file(outfilename)

# Set output root file: this is a separate root file in which your
# analysis module can store anything such as histograms, your own TTree, etc.
my_proc.set_ana_output_file("aaa.root")
my_proc.set_output_file("bbb.root")

#my_proc.set_output_rootdir("scanner")
# Create analysis class instance. For this example, ana_base.
# To show how one can run multiple analysis modules at once,
# we make multiple ana_base instance.

compAna=fmwk.ExecuteCompression()
compAna.SetSaveOutput(False)
compAna.SetUseSimch(True)
#add Compression Algorithm
compAlgo = compress.CompressionAlgosncompress()
compAlgo.SetDebug(False)
compAlgo.SetVerbose(True)
compAlgo.SetFillTree(False)
compAlgo.SetBlockSize(64)
compAlgo.SetBaselineThresh(0.75)
compAlgo.SetVarianceThresh(1.00)
thresh = float(sys.argv[-1])
compAlgo.SetCompressThresh(-thresh,thresh,thresh)
compAlgo.SetMaxADC(4095)
compAlgo.SetUVYplaneBuffer(30,55,15,20,15,10);

compAna.SetCompressAlgo(compAlgo)

#add study Algorithm
compStudy = compress.CompressionStudyHits()
compStudy.setThreshold(5.)
compStudy.setConsecutiveTicks(3)
compStudy.SetVerbose(True)

#add viewer Algorithm
compView  = compress.ViewCompression()

#add IDE study Algorithm
compIDE = compress.CompressionStudyIDEs()
compIDE.SetVerbose(True)

compAna.SetCompressAlgo(compAlgo)
compAna.SetCompressStudy(compStudy)
compAna.SetIDEStudy(compIDE)
compAna.SetCompressViewer(compView)

# Add analysis modules to the processor

my_proc.add_process(compAna)

nextevent = False;

fig, axIn  = plt.subplots(nrows=1,figsize=(15,5))

axIn_IDE  = axIn.twinx()

while my_proc.process_event():
    print "Waveforms in event: ",compAna.GetNumWFs()
    chan = 0
    for x in xrange(compAna.GetNumWFs()):
        if (nextevent == True):
            nextevent = False;
            break;
        compAna.ApplyCompression(chan)
        if (compView.GetNumOutWFs() >= 1):
            print 'Evt: %i  Chan: %i  Plane: %i  --> Out WFs: %i'%(compView.GetEvtNum(),compView.GetChan(),compView.GetPlane(),compView.GetNumOutWFs())
            axIn.clear()
            axIn_IDE.clear()
            inWF  = np.array(compView.GetADCs(1))
            IDE   = np.array(compView.GetIDE())
            axIn.set_ylabel('ADCs',fontsize=16,color='b')
            axIn.set_title('RawDigit Waveform. Evt: %i  Chan: %i  Plane: %i'
                           %(compView.GetEvtNum(),compView.GetChan(),compView.GetPlane()))
            axIn_IDE.set_ylabel('Energy Dep [MeV]',fontsize=16,color='r')
            axIn.plot(inWF,'b-',linewidth=2)
            for t1 in axIn.get_yticklabels():
                t1.set_color('b')
            axIn_IDE.plot(IDE,'r-',linewidth=2)
            for t2 in axIn_IDE.get_yticklabels():
                t2.set_color('r')
            axIn.set_xlim([0,len(inWF)])
            #axOut.set_ylim([axIn.get_ylim()[0],axIn.get_ylim()[1]])
            fig.canvas
            #fig.canvas.draw()
            fig.show()

            usrinput = raw_input("Hit Enter: next evt  ||  q: exit viewer || n: next event\n")
            if ( usrinput == "q" ):
                sys.exit(0)
            elif ( usrinput == "n" ):
                nextevent = True;
                chan = 0
            elif ( usrinput.isdigit() ):
                chan += int(usrinput)
            else:
                chan += 1
        else:
            chan += 1

                
# done!

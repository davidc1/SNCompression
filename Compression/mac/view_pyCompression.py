# Load libraries
import sys, os
import ROOT
from larlite import larlite as fmwk
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

useSimch = False

compAna=fmwk.ExecuteCompression()
compAna.SetSaveOutput(False)
compAna.SetUseSimch(useSimch)
#add Compression Algorithm
compAlgo = compress.CompressionAlgosncompress()
compAlgo.SetDebug(False)
compAlgo.SetVerbose(False)
compAlgo.SetFillTree(True)
compAlgo.SetBlockSize(64)
compAlgo.SetBaselineThresh(5.0)
compAlgo.SetVarianceThresh(5.0)
thresh = float(sys.argv[-1])
compAlgo.SetCompressThresh(-thresh,thresh,thresh)
compAlgo.SetMaxADC(4095)
compAlgo.SetUVYplaneBuffer(30,30,30,30,30,30)
compAna.SetCompressAlgo(compAlgo)

#add study Algorithm
compStudy = compress.CompressionStudyBaseline()
#compStudy.setThreshold(5.)
#compStudy.setConsecutiveTicks(3)
compStudy.SetVerbose(True)

#add viewer Algorithm
compView  = compress.ViewCompression()
compView.suppressBaseline(True)

compAna.SetCompressAlgo(compAlgo)
compAna.SetCompressStudy(compStudy)
compAna.SetCompressViewer(compView)

# Add analysis modules to the processor

my_proc.add_process(compAna)

nextevent = False;

fig = plt.figure(figsize=(15,6)) 

while my_proc.process_event():
    print "Waveforms in event: ",compAna.GetNumWFs()
    chan = 0
    for chan in xrange(compAna.GetNumWFs()):
        if (nextevent == True):
            nextevent = False;
            break;
        compAna.ApplyCompression(chan)
        if (compView.GetPlane() != 2) : continue
        if (compView.GetNumOutWFs() >= 1):
            print 'Evt: %i  Chan: %i  Plane: %i  --> Out WFs: %i'%(compView.GetEvtNum(),compView.GetChan(),compView.GetPlane(),compView.GetNumOutWFs())
            fig.gca().clear()
            inWF  = np.array(compView.GetInADCs())
            outWF_v = compView.GetOutADCs()
            outWF_t = compView.GetOutTicks()
            plt.ylabel('ADCs',fontsize=16,color='b')
            plt.title('Compression Output. Evt: %i  Chan: %i  Plane: %i'
                      %(compView.GetEvtNum(),compView.GetChan(),compView.GetPlane()))
            plt.plot(inWF,'b-',linewidth=2)

            for wf_n in xrange(outWF_t.size()):
                out_wf = outWF_v.at(wf_n)
                out_tick = outWF_t.at(wf_n)
                tick_range = np.linspace(out_tick, out_tick + out_wf.size() - 1, out_wf.size() )
                plt.plot(tick_range, np.array(out_wf) + 1, color='r',linewidth=2)
                
            plt.xlim([0,len(inWF)])
            plt.grid()

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

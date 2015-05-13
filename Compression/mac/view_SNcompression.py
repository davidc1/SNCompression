# Load libraries
import sys, os
from ROOT import *
from ROOT import gSystem
from ROOT import larlite as fmwk
from ROOT import compress

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
#my_proc.set_io_mode(fmwk.storage_manager.kBOTH)
#my_proc.set_io_mode(fmwk.storage_manager.kWRITE)
my_proc.set_io_mode(fmwk.storage_manager.kREAD)


for x in xrange(len(sys.argv)-2):
    my_proc.add_input_file(sys.argv[x+1])


# Set output root file: this is a separate root file in which your
# analysis module can store anything such as histograms, your own TTree, etc.
my_proc.set_ana_output_file("aaa.root")
my_proc.set_output_file("bbb.root")

compAna=fmwk.ViewCompression()
compAna.suppressBaseline(True)
#add Compression Algorithm
print "hello"
compAlgo = compress.CompressionAlgosncompress()
print "hello"
compAlgo.SetDebug(False)
compAlgo.SetVerbose(True)
compAlgo.SetFillTree(False)
compAlgo.SetBlockSize(64)
compAlgo.SetBaselineThresh(0.75)
compAlgo.SetVarianceThresh(1)
thresh = float(sys.argv[-1])
compAlgo.SetCompressThresh(-thresh,thresh,thresh)
compAlgo.SetMaxADC(4095)
compAlgo.SetUVYplaneBuffer(30,55,15,20,15,10);
compAna.SetCompressAlgo(compAlgo)

my_proc.add_process(compAna)

gStyle.SetOptStat(0)
gStyle.SetTitleFontSize(0.06)
gStyle.SetTitleOffset(0.4,"X")
gStyle.SetTitleSize(0.06,"X")
gStyle.SetTitleOffset(0.8,"Y")
gStyle.SetTitleSize(0.06,"Y")
gStyle.SetLabelSize(0.06,"X")
gStyle.SetLabelSize(0.06,"Y")

c1 = TCanvas("c1","c1",900,900)
c1.Divide(1,2)

while my_proc.process_event():

    for x in range(compAna.GetNumWFs()):
        compAna.processWF()
        if (compAna.GetNumOutWFs() >= 0):
            print "%i Waveforms found"%compAna.GetNumOutWFs()
            for pad in xrange(1,3,1):
                c1.cd(pad)
                compAna.GetHistos(pad).Draw()
                basehisto = compAna.GetBaseHisto()
                basehisto.SetLineColor(2)
                basehisto.Draw("same")
                varhisto = compAna.GetVarHisto()
                varhisto.SetLineColor(3)
                varhisto.Draw("same")
            c1.Update()

            usrinput = raw_input("Hit Enter: next evt  ||  q: exit viewer\n")
            if ( usrinput == "q" ):
                sys.exit(0)


    

# done!

# Load libraries
import ROOT, sys, os
from ROOT import *
from ROOT import larlite as fmwk

filename = sys.argv[1]

my_proc = fmwk.ana_processor()

my_proc.set_io_mode(fmwk.storage_manager.kREAD)

my_proc.add_input_file(filename)

#is there a way to disable ana_proc from creating an output file at all?
my_proc.set_ana_output_file("")

my_ana = fmwk.WFViewer()
my_ana.setCoordinatesInCm(False)

my_proc.add_process(my_ana)


gStyle.SetOptStat(0)
gStyle.SetTitleFontSize(0.1)
gStyle.SetTitleOffset(1,"X")
gStyle.SetTitleSize(0.04,"X")
gStyle.SetTitleOffset(1,"Y")
gStyle.SetTitleSize(0.04,"Y")
gStyle.SetLabelSize(0.04,"X")
gStyle.SetLabelSize(0.04,"Y")
gStyle.SetLabelSize(0.04,"Z")
gStyle.SetOptLogz(0)

#my_proc.run(0)

c=TCanvas("c","Wire v. Time Hit Viewer",900,700)
c.Divide(1,1)
gPad.SetLogz()

while my_proc.process_event():

    currentview = 2;
    #First fill the 6 pads on the main canvas with stuff
    for pad in xrange(1,2,1):
        
        c.cd(pad)
        histo = my_ana.GetHisto_Hits(int(currentview))
        histo.Draw("COLZ")
    
        currentview = currentview + 1
        c.Update()
    

    usrinput = raw_input("Hit Enter: next evt  ||  q: exit viewer\n")
    if ( usrinput == "q" ):
        sys.exit(0)


# done!

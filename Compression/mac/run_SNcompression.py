#
# Example PyROOT script to run analysis module, ana_base.
# The usage is same for inherited analysis class instance.
#

# Load libraries
import os, ROOT, sys
from ROOT import gSystem
from ROOT import larlite as fmwk
from ROOT import compress

# Now import ana_processor & your class. For this example, ana_base.

infilename = sys.argv[1]

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
#my_proc.set_io_mode(fmwk.storage_manager.kBOTH)
#my_proc.set_io_mode(fmwk.storage_manager.kWRITE)
my_proc.set_io_mode(fmwk.storage_manager.kREAD)


# Set input root file: this is decoder output root file.
# This time, we use a sample file prepared.
#my_proc.add_input_file("./../../../NevisDecoder/Decoder/mac/xmit_subrun_2014_01_13_1_trigger.root")
# Set input root file
for x in xrange(len(sys.argv)-2):
    my_proc.add_input_file(sys.argv[x+1])

#set output data file
#my_proc.set_output_file(outfilename)

# Set output root file: this is a separate root file in which your
# analysis module can store anything such as histograms, your own TTree, etc.
my_proc.set_ana_output_file("Anaoutput.root")
my_proc.set_output_file("compressedWFs.root")

#my_proc.set_output_rootdir("scanner")
# Create analysis class instance. For this example, ana_base.
# To show how one can run multiple analysis modules at once,
# we make multiple ana_base instance.

compAna=fmwk.ExecuteCompression()
compAna.SetSaveOutput(False)

#add Compression Algorithm
compAlgo = compress.CompressionAlgosncompress()
compAlgo.SetDebug(False)
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

compAna.SetCompressAlgo(compAlgo)
compAna.SetCompressStudy(compStudy)

# Add analysis modules to the processor

my_proc.add_process(compAna)

# Let's run it.

my_proc.run(0,2)

# done!

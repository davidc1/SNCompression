# Load libraries
import sys, os
import ROOT
from larlite import larlite as fmwk
from ROOT import compress

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
my_proc.set_ana_output_file("buffer_optimize.root")
my_proc.set_output_file("compressedWFs.root")

#my_proc.set_output_rootdir("scanner")
# Create analysis class instance. For this example, ana_base.
# To show how one can run multiple analysis modules at once,
# we make multiple ana_base instance.

compAna=fmwk.ExecuteCompression()
compAna.SetSaveOutput(False)
compAna.SetUseSimch(False)
#add Compression Algorithm
compAlgo = compress.CompressionAlgosncompress()
compAlgo.SetDebug(False)
compAlgo.SetVerbose(False)
compAlgo.SetFillTree(False)
compAlgo.SetBlockSize(64)
compAlgo.SetBaselineThresh(0.75)
compAlgo.SetVarianceThresh(1.00)
thresh = 5.;
compAlgo.SetCompressThresh(-thresh,thresh,thresh)
compAlgo.SetMaxADC(4095)
buff = int(sys.argv[-1])
#compAlgo.SetUVYplaneBuffer(buff,buff,buff,buff,buff,buff);
compAlgo.SetUVYplaneBuffer(55,18,47,30,20,20)
compAna.SetCompressAlgo(compAlgo)

#add HIT study Algorithm
compStudy = compress.CompressionStudyBaseline()

compAna.SetCompressAlgo(compAlgo)
compAna.SetCompressStudy(compStudy)

# Add analysis modules to the processor

my_proc.add_process(compAna)

# Let's run it.

my_proc.run(0,10)

# done!

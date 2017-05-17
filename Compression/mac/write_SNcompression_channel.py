# Load libraries
import sys, os
import ROOT
from larlite import larlite as fmwk
from ROOT import compress

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
my_proc.set_io_mode(fmwk.storage_manager.kBOTH)
#my_proc.set_io_mode(fmwk.storage_manager.kWRITE)
#my_proc.set_io_mode(fmwk.storage_manager.kREAD)

#set output data file
#my_proc.set_output_file(outfilename)

# Set output root file: this is a separate root file in which your
# analysis module can store anything such as histograms, your own TTree, etc

##################################################
##### TO SET OUTPUT FILE FROM COMMAND LINE #######
##      UNCOMMENT THE FOLLOWING 4 LINES:       ###

for x in xrange(len(sys.argv)-6):
    my_proc.add_input_file(sys.argv[x+1])
    my_proc.set_output_file(sys.argv[-5]) #this is the output_ana root file, third from last entry in the command line
   # my_proc.set_output_file("compressed_WFs.root")      #sys.argv[-7])   #this is the compressed waveforms root file, second from last entry in the command line

#### to run: $ python $LARLITE_USERDEVDIR/SNCompression/Compression/mac/run_SNcompression.py /PATH_TO_output_ana.root /PATH_TO_compressedWFs.root [ADC threshold]
##########################################
## AND COMMENT OUT THE FOLLOWING 4:     ##
##########################################
#for x in xrange(len(sys.argv)-2):
#    my_proc.add_input_file(sys.argv[x+1])
#    my_proc.set_ana_output_file("SNoutput_ana.root") #creates file in working directory
#    my_proc.set_output_file("compressedWFs.root") #creates file in working directory

#########################################
#### to run: $ python $LARLITE_USERDEVDIR/SNCompression/Compression/mac/run_SNcompression.py [ADC threshold]


#my_proc.set_output_rootdir("scanner")
# Create analysis class instance. For this example, ana_base.
# To show how one can run multiple analysis modules at once,
# we make multiple ana_base instance.

compAna=fmwk.ExecuteCompression()
compAna.SetSaveOutput(True)
compAna.SetUseSimch(False)
#add Compression Algorithm
compAlgo = compress.CompressionAlgosncompress()
compAlgo.SetDebug(False)
compAlgo.SetVerbose(False)
compAlgo.SetFillTree(False)
compAlgo.SetBlockSize(64)
compAlgo.SetBaselineThresh(2.0)
compAlgo.SetVarianceThresh(2.0)
"""
threshU = float(sys.argv[-5])
threshV = float(sys.argv[-3])
threshY = float(sys.argv[-1])
"""
ubU = int(sys.argv[-3])
ubV = int(sys.argv[-2])
ubY = int(sys.argv[-1])

print "set polarities"

compAlgo.SetCompressThresh(sys.argv[-4])
compAlgo.SetPolarity(ubU,ubV,ubY)

print "read in the input file into SetCompressThresh"

compAlgo.SetMaxADC(4095)
compAlgo.SetUVYplaneBuffer(7,8,7,8,7,8,) #55,18,47,30,20,20)
#compAlgo.SetUVYplaneBuffer(30,55,15,20,15,10);
compAna.SetCompressAlgo(compAlgo)

#add HIT study Algorithm
compStudy = compress.CompressionStudyHits()
compStudy.setThreshold(5.)
compStudy.setConsecutiveTicks(3)

#add IDE study Algorithm
compIDE = compress.CompressionStudyIDEs()
compIDE.SetVerbose(False)

#####compAna.SetCompressAlgo(compAlgo)
#compAna.SetCompressStudy(compStudy)
#compAna.SetIDEStudy(compIDE)

# Add analysis modules to the processor

my_proc.add_process(compAna)

# Let's run it.

my_proc.run() #(first event, how many events)

# done!

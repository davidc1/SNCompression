# Load libraries
import sys, os
from ROOT import *
from ROOT import gSystem
from ROOT import larlite as fmwk
from ROOT import compress

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
my_proc.set_io_mode(fmwk.storage_manager.kBOTH)
#my_proc.set_io_mode(fmwk.storage_manager.kWRITE)
#my_proc.set_io_mode(fmwk.storage_manager.kREAD)


for x in xrange(len(sys.argv)-1):
    my_proc.add_input_file(sys.argv[x+1])

# Set output root file: this is a separate root file in which your
# analysis module can store anything such as histograms, your own TTree, etc.
my_proc.set_output_file("hits.root")
my_proc.set_ana_output_file("")


my_ana=fmwk.HitMaker()

# Add analysis modules to the processor
my_proc.add_process(my_ana)

# Let's run it.
my_proc.run(0,1)

# done!

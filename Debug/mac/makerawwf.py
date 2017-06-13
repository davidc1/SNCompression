#
# Example PyROOT script to run analysis module, ana_base.
# The usage is same for inherited analysis class instance.
#

# Load libraries
import os, ROOT, sys
from larlite import larlite as fmwk
from ROOT import std

# Now import ana_processor & your class. For this example, ana_base.
#from ROOT import larlite as fmwk#

# Create ana_processor instance
my_proc=fmwk.ana_processor()

# Specify IO mode
#my_proc.set_io_mode(fmwk.storage_manager.kREAD)
my_proc.set_io_mode(fmwk.storage_manager.kBOTH)

for x in xrange(len(sys.argv)-1):
    my_proc.add_input_file(sys.argv[x+1])

my_proc.set_ana_output_file("")
my_proc.set_output_file("fakewf.root")


ana = fmwk.MakeFakeWF()

v = std.vector('short')()
v += [2048,2048,2047,2047,2048,2048,2048,2050,2070,2100,2150,2047,2042,2048,2047,2048]

ana.addWaveform(1000,v)
ana.addWaveform(3000,v)
ana.addWaveform(5000,v)

#my_proc.enable_filter(True)

my_proc.set_data_to_write(fmwk.data.kRawDigit,'daq')

# set the number of pi0s requested
my_proc.add_process(ana)

#my_proc.enable_filter(True)
my_proc.run(0,1)
sys.exit(0);

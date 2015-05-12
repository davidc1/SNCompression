# Load libraries
import ROOT, sys, os
from ROOT import *

# Now import ana_processor & your class. For this example, ana_base.
gSystem.Load("libAnalysis")
gSystem.Load("libLArUtil")
gSystem.Load("libSNCompression")

filename = sys.argv[1]

my_proc = larlight.ana_processor()

my_proc.set_verbosity(larlight.MSG.DEBUG)

my_proc.set_io_mode(larlight.storage_manager.READ)

#my_proc.set_input_rootdir("scanner")

my_proc.add_input_file(filename)

larlight.storage_manager.get().set_in_rootdir("scanner")
larlight.storage_manager.get().set_data_to_read(larlight.DATA.FFTHit)
larlight.storage_manager.get().set_data_to_read(larlight.DATA.UserInfo,False)

#is there a way to disable ana_proc from creating an output file at all?
my_proc.set_ana_output_file("")

my_ana = larlight.WFSaver()

my_proc.add_process(my_ana)

my_proc.run(0,1)

# done!

import sys

if len(sys.argv) < 2:
    msg  = '\n'
    msg += "Usage 1: %s $INPUT_ROOT_FILE\n" % sys.argv[0]
    msg += '\n'
    sys.stderr.write(msg)
    sys.exit(1)


from larlite import larlite as fmwk

# Create ana_processor instance
my_proc = fmwk.ana_processor()

# Set input root file
for x in xrange(len(sys.argv)-1):
    my_proc.add_input_file(sys.argv[x+1])

# Specify IO mode
my_proc.set_io_mode(fmwk.storage_manager.kBOTH)

# Specify analysis output root file name
my_proc.set_ana_output_file("");

# Specify data output root file name
my_proc.set_output_file("sn_hits.root")

hitmaker = fmwk.SNHitReco()
hitmaker.SetWireProducer("caldata")
hitmaker.SetHitProducer("snhit")
my_proc.add_process(hitmaker)

my_proc.set_data_to_write(fmwk.data.kHit,'snhit')
my_proc.set_data_to_write(fmwk.data.kWire,'caldata')
#my_proc.set_data_to_write(fmwk.data.kRawDigit,'daq')



print
print  "Finished configuring ana_processor. Start event loop!"
print

my_proc.run()
#my_proc.run()

sys.exit()


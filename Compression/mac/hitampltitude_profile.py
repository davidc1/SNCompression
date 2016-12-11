import os,sys
import ROOT
from larlite import larlite as fmwk
import numpy as np
import matplotlib.pyplot as plt

if (len(sys.argv) != 2):
    print 'ERROR: run as follows:'
    print 'python hitamplotude_profile.py WIRE_FILE.ROOT'
    print 'script currently accepts a single input file'
    sys.exit(0)

# vectors where to store ADC amplitudes
ADC_Y_max_v = []
ADC_V_max_v = []
ADC_U_max_v = []

ADC_Y_min_v = []
ADC_V_min_v = []
ADC_U_min_v = []

# open ROOT file
fin = ROOT.TFile(sys.argv[-1])
# open TTree
t = fin.Get('wire_caldata_tree')

# loop over all entries in TTree (each entry one event)
for n in xrange(t.GetEntries()):

    # get nth entry
    t.GetEntry(n)

    # get larlite wire branch. This is an instance
    # of event_wire which you can find in core/DataFormat/wire.h
    # this won't work if larlite is not setup
    # wire_v is a C++ std::vector so you can use all
    # the functions you know about std::vectors on it
    wire_v = t.wire_caldata_branch

    # loop through all wires
    for i in xrange(wire_v.size()):

        wire = wire_v.at(i)

        # now you have the wire object
        # the goal is to find, for each wire, the
        # maximum (or minimum, it is up to you) ADC value for that wire
        # and plot it
        # to figure out how to navigate the wire object go read
        # core/DataFormat/wire.h

        # the plane number is a bit tricky, so you get that like this:
        plane = int(wire.View())

        ROIs = wire.SignalROI()

        ranges = ROIs.get_ranges()

        # copy of maximum ADCs for this wire
        maxadcs = []
        minadcs = []

        for j in xrange(ranges.size()):

            data = ranges.at(j).data()

            adcmax = -1
            adcmin = 4095
            for adc in data:
                if (adc > adcmax): adcmax = adc
                if (adc < adcmin): adcmin = adc
            maxadcs.append( adcmax )
            minadcs.append( adcmin )

        if (plane == 2):
            for adc in maxadcs:
                ADC_Y_max_v.append( adc )
            for adc in minadcs:
                ADC_Y_min_v.append( adc )
        if (plane == 1):
            for adc in maxadcs:
                ADC_V_max_v.append( adc )
            for adc in minadcs:
                ADC_V_min_v.append( adc )
        if (plane == 0):
            for adc in maxadcs:
                ADC_U_max_v.append( adc )
            for adc in minadcs:
                ADC_U_min_v.append( adc )

# we filled the vectors, let's plot them
# how to make histograms using matplotlib?
# se here: http://matplotlib.org/1.2.1/examples/api/histogram_demo.html
# or our many examples here: https://github.com/NevisUB/Tutorials/tree/master/matplotlib

# change font-size once for entire plotting routine
plt.rcParams.update({'font.size': 20})

# Plane 2:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(0,200,100) # arguments are min, max, nbins
plt.hist( ADC_Y_max_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 2')
plt.xlabel('Max ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([0,200])
#plt.ylim([X,X])
plt.legend()
plt.show()

# Plane 1:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(0,100,100) # arguments are min, max, nbins
plt.hist( ADC_V_max_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 1')
plt.xlabel('Max ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([0,100])
#plt.ylim([X,X])
plt.legend()
plt.show()

# Plane 0:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(0,100,100) # arguments are min, max, nbins
plt.hist( ADC_U_max_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 0')
plt.xlabel('Max ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([0,100])
#plt.ylim([X,X])
plt.legend()
plt.show()

# Plane 2:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(-100,0,100) # arguments are min, max, nbins
plt.hist( ADC_Y_min_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 2')
plt.xlabel('Min ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([-100,0])
#plt.ylim([X,X])
plt.legend()
plt.show()

# Plane 1:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(-100,0,100) # arguments are min, max, nbins
plt.hist( ADC_V_min_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 1')
plt.xlabel('Min ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([-100,0])
#plt.ylim([X,X])
plt.legend()
plt.show()

# Plane 0:
fig = plt.figure(figsize=(10,10))
BINS = np.linspace(-100,0,100) # arguments are min, max, nbins
plt.hist( ADC_U_min_v, bins=BINS, histtype='stepfilled', color='r', label='Plane 0')
plt.xlabel('Min ADC amplitude')
plt.title('Pulse Height Distribution for SN output waveforms',fontsize=16)
plt.grid()
plt.xlim([-100,0])
#plt.ylim([X,X])
plt.legend()
plt.show()
    

    

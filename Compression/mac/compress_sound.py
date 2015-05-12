#
# Example PyROOT script to run analysis module, ana_base.
# The usage is same for inherited analysis class instance.
#

# Load libraries
import os, ROOT, sys
from ROOT import gSystem
from ROOT import compress

import matplotlib.pyplot as plt
import numpy as np
import wave

gSystem.Load("libAnalysis")
gSystem.Load("libLArUtil")
gSystem.Load("libSNCompression")

# Now import ana_processor & your class. For this example, ana_base.
from ROOT import larlight as fmwk


infilename = sys.argv[1]

#sound recording
spf = wave.open(infilename,'r')
#get frame rate
frameRate = spf.getframerate()
#Extract Raw Audio from Wav File
signal = spf.readframes(-1)
signal = np.fromstring(signal,np.uint8)
#get correct time coordinate
time = np.linspace(0,len(signal)/frameRate,len(signal))

sound = ROOT.vector('unsigned short')()
for x in range(len(signal)):
    sound.push_back(int(signal[x]))

#add Compression Algorithm
compAlgo = compress.CompressionAlgoThresh()
compAlgo.SetVerbose(False)
compAlgo.SetDebug(False)
compAlgo.SetThreshold(3)

#feed in sound
compAlgo.ApplyCompression(sound)
#outputSound = compAlgo.GetOutputWFs()
outputTimes = compAlgo.GetOutputWFTimes()
'''
print outputTimes.at(0)
print outputTimes.size()
sounds = ROOT.vector(ROOT.vector('int')())
sound1 = ROOT.vector('int')()
sound1.push_back(0)
sounds.push_back(sounds1)
#print outputSound.size()
'''
print "Found {0} spikes.".format(len(outputTimes))

plt.figure(1)
plt.title('Sound Wave')
plt.xlabel('Time [Seconds]')
plt.ylabel('Amplitude')
for i in range(int(outputSound.size())):
    times = np.linspace(outputTimes.at(i),outputTimes.at(i)+outputSound.at(i).size(),outputSound.at(i).size())
    plt.plot(times,outputSound[i])
plt.show()


#ifndef COMPRESSIONALGOONETHRESH_CXX
#define COMPRESSIONALGOONETHRESH_CXX

#include "CompressionAlgoOneThresh.h"

namespace compress {
  
  void CompressionAlgoOneThresh::ApplyCompression(const std::vector<short> &waveform, int mode){

    //Algorithm Computation starts here!
    PrepareAlgo();

    //assign the waveform vector for this specific instance of the algorithm
    SetInWF(waveform);

    //calculate the baseline from the firxt X samples
    FindBaseline();

    //loop over wf...if a segment moves above threshold -> add to output vector
    //keep track on whether already triggered or not
    bool active = false;
    int startTick = 0;

    // iterators pointing to beginning and end of output wf
    compress::tick tstart;
    compress::tick tend;
    // iterator
    compress::tick t;
    for (t = _InWF.begin(); t < _InWF.end(); t++){
      if ( _debug ) {
        std::string x;
        std::cout << "next tick? Enter any key:" << std::endl;
        std::cin >> x;
        std::cout << "thanks!" << std::endl;
      }

      if ( _verbose ) {
        std::cout << "current ADC tick: " << *t << std::endl;
        std::cout << "Baseline: " << _Baseline << std::endl;
        std::cout << "Active? " << active << std::endl;
      }

      // if the baseline-subtracted ADC tick is greater than
      // the threshold in absolute value -> fire compression!
      if ( fabs( ( *t - _Baseline) ) > _Threshold[mode] ){
        if ( !active ) { tstart = t; } // if it is the first Tick above baseline -> start a new waveform
        active = true;
      }
      else {
        if ( (active) and (t > tstart) ){
	  tend = t;
	  _timeRange.push_back(std::make_pair(tstart,tend));
          active = false;
        }
      }
    }// for all ticks
    return;
  }

  void CompressionAlgoOneThresh::FindBaseline(){

    _Baseline = 0.;
    // tick-iterator
    compress::tick t;
    for (t = _InWF.begin(); t < _InWF.begin()+4; t++)
      _Baseline += *t;
    _Baseline /= 4;

    return;
  }
}

#endif

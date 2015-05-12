/**
 * \file CompressionAlgoOneThresh.h
 *
 * \ingroup SNCompression
 * 
 * @author Yun-Tse Tsai
 */

/** \addtogroup SNCompression

    @{*/
#ifndef COMPRESSIONALGOONETHRESH_H
#define COMPRESSIONALGOONETHRESH_H

#include "CompressionAlgoBase.h"
#include <math.h>

namespace compress {

  /**
     \class CompressionAlgoOneThresh
     A Class where to write a compressiona algorithm for TPC wire signals.
  */
  class CompressionAlgoOneThresh : public CompressionAlgoBase {
    
  public:

    /// prepare aglo
    void PrepareAlgo() {};
    
    /// Function where compression is performed
    void ApplyCompression(const std::vector<short> &waveform, int mode);

    /// Function to set input waveform to be compressed
    void SetInWF(std::vector<short> waveform) { _InWF = waveform; }

    // Function to find WF baseline...
    void FindBaseline();

    /// Function to set Threshold
    void SetThreshold( int thr0, int thr1, int thr2 ) { _Threshold[0] = thr0; _Threshold[1] = thr1; _Threshold[2] = thr2; }

  protected:

    /// Baseline value
    double _Baseline;
    /// Threshold for cut
    double _Threshold[3];

  };

}

#endif
/** @} */ // end of doxygen group 


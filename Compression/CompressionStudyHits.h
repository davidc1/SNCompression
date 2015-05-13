/**
 * \file compressionstudyhits.h
 *
 * \ingroup SNCompression
 * 
 * @author david
 */

/** \addtogroup SNCompression

    @{*/
#ifndef COMPRESSIONSTUDYHITS_H
#define COMPRESSIONSTUDYHITS_H

#include "CompressionStudyBase.h"
#include <math.h>

namespace compress {

  /**
     \class compressionstudyhits
     A Class where to write a compressiona algorithm for TPC wire signals.
  */
  class CompressionStudyHits : public CompressionStudyBase {
    
  public:

    CompressionStudyHits();

    /// Function where compression is performed
    void StudyCompression(const std::vector<short> &in,
			  const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			  const int pl);

    /// Close Algorithm (maybe to write trees & such)
    void EndProcess(TFile* fout=nullptr);

    /// Set threshold
    void setThreshold(double t) { _threshold = t; }

  protected:


    void findHits(const std::pair<tick,tick>& range, bool in);

    bool PassThreshold(double thisADC, double base);

    double _threshold;
    
    // TTree and variable for TTree
    TTree *_in_study_tree;
    TTree *_out_study_tree;
    double _hit_peak; // hit max Q for waveforms
    double _hit_area; // hit area Q for waveforms
    int    _pl; // Plane

  };

}

#endif
/** @} */ // end of doxygen group 


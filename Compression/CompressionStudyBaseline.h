/**
 * \file CompressionStudyBaseline.h
 *
 * \ingroup SNCompression
 * 
 * @author david
 */

/** \addtogroup SNCompression

    @{*/
#ifndef COMPRESSIONSTUDYBASELINE_H
#define COMPRESSIONSTUDYBASELINE_H

#include "CompressionStudyBase.h"
#include <math.h>
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"

namespace compress {

  /**
     \class compressionstudyhits
     A Class where to write a compressiona algorithm for TPC wire signals.
  */
  class CompressionStudyBaseline : public CompressionStudyBase {
    
  public:

    CompressionStudyBaseline();

    /// Function where compression is performed
    void StudyCompression(const std::vector<short> &in,
			  const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			  const int pl);

    /// Close Algorithm (maybe to write trees & such)
    void EndProcess(TFile* fout=nullptr);


  protected:

    /// Setup model for baseline fitting
    void PrepareLinearModel();
    
    
    void EvaluateBaseline(const std::pair<tick,tick>& outrange);
    
    // TTree and variable for TTree
    TTree *_base_tree;
    double _base_pre;   // baseline measured from first 3 ticks
    double _base_post;  // baseline measured from last 3 ticks
    double _slope_pre;  // slope measured from first 3 ticks
    double _slope_post; // slope measured from last 3 ticks
    int    _pl;         // Plane

    // Model for fitting
    TF1 *_model;

    // waveform begin
    compress::tick _begin;
    
  };
  
}

#endif
/** @} */ // end of doxygen group 


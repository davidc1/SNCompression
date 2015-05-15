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

    /// Set number of consecutive ticks needed to save a hit
    void setConsecutiveTicks(int c) { _consecutive = c; }

  protected:


  void matchHits(const std::pair<tick,tick>& inrange,
		 const std::vector<std::pair<tick,tick> >& outranges);

  void findHits(const std::pair<tick,tick>& range, bool in);

  // has the tick we are examining been saved in the output? return boolean
  bool isTickInOutput(const compress::tick& t, const std::vector<std::pair<tick,tick> >& outranges, size_t& currentPair);
  
  bool PassThreshold(double thisADC, double base);

  // threshold that needs to be met to be in "hit" region
  double _threshold;
  // number of consecutive ticks above threshold required to save a hit
  int _consecutive;
  
  // TTree and variable for TTree
  TTree *_in_study_tree;
  TTree *_out_study_tree;
  double _baseline; // baseline used for this tick
  double _hit_peak; // hit max Q for waveforms
  double _hit_area; // hit area Q for waveforms
  double _hit_peakOut; // hit max Q for output waveforms
  double _hit_areaOut; // hit area Q for output waveforms
  int    _pl;       // Plane
  int    _isSaved;  // is the tick saved in the output wf?
  
  };

}

#endif
/** @} */ // end of doxygen group 


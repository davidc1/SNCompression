/**
 * \file CompressionStudyIDEs.h
 *
 * \ingroup DavidAna
 * 
 * @author David Caratelli
 */

/** \addtogroup ClusterRecoUtil

    @{*/
#ifndef COMPRESSIONSTUDYIDES_H
#define COMPRESSIONSTUDYIDES_H

#include "CompressionAlgoBase.h"
#include "TTree.h"

namespace compress {

  /**
     \class CMAlgoBase
     ...
  */
  class CompressionStudyIDEs {
    
  public:
    
    /// Default constructor
    CompressionStudyIDEs();

    /// Default destructor
    ~CompressionStudyIDEs(){}

    /// Close Algorithm (maybe to write trees & such)
    void EndProcess(TFile* fout=nullptr);
 
    /// Setter function for verbosity
    void SetVerbose(bool doit=true) { _verbose = doit; }

    /// Setter function for debug mode
    void SetDebug(bool doit=true) { _debug = doit; }

    /// Function where study is performed
    void StudyCompression(const std::vector<std::pair<unsigned short, double> >& IDEs,
			  const std::pair<compress::tick,compress::tick>& range,
			  const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			  const int pl, const int ch, const int evt);

  protected:

    /// Boolean to choose verbose mode.
    bool _verbose;

    /// Boolean to choose debug mode.
    bool _debug;

    /// TTree where to save output info
    TTree* _ide_study;
    int _pl; /// plane info
    int _ch; /// channel info
    int _evt; /// event info
    double _ideE; /// energy in IDE pulse
    double _ideEout; /// energy in output IDE pulse
    double _idePeak; /// peak energy for this IDE pulse
    int _start; /// start tick of IDE pulse
    int _end; /// end tick of ide pulse

    bool isTickInOutput(const compress::tick& t, const std::vector<std::pair<tick,tick> >& outranges, size_t& currentPair);

  };

}

#endif
/** @} */ // end of doxygen group 


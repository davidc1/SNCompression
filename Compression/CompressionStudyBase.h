/**
 * \file CompressionStudyBase.h
 *
 * \ingroup DavidAna
 * 
 * @author David Caratelli
 */

/** \addtogroup ClusterRecoUtil

    @{*/
#ifndef COMPRESSIONSTUDYBASE_H
#define COMPRESSIONSTUDYBASE_H

#include <iostream>
#include "LArUtil/Geometry.h"
#include "CompressionAlgoBase.h"
#include <vector>

namespace compress {

  /**
     \class CMAlgoBase
     ...
  */
  class CompressionStudyBase {
    
  public:
    
    /// Default constructor
    CompressionStudyBase();

    /// Default destructor
    virtual ~CompressionStudyBase(){}
    /// Open (maybe to write trees & such)
    virtual void ProcessBegin() {}

    /// Close Algorithm (maybe to write trees & such)
    virtual void EndProcess(TFile* fout=nullptr) {}
 
    /// Setter function for verbosity
    virtual void SetVerbose(bool doit=true) { _verbose = doit; }

    /// Setter function for debug mode
    virtual void SetDebug(bool doit=true) { _debug = doit; }

    /// Function where study is performed
    virtual void StudyCompression(const std::vector<short> &in,
				  const std::vector<std::pair< compress::tick, compress::tick> > &ranges,				  
				  const int pl);

  protected:

    /// Boolean to choose verbose mode.
    bool _verbose;

    /// Boolean to choose debug mode.
    bool _debug;

  };

}

#endif
/** @} */ // end of doxygen group 


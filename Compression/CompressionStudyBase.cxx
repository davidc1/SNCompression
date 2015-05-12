#ifndef COMPRESSIONSTUDYBASE_CXX
#define COMPRESSIONSTUDYBASE_CXX

#include "CompressionStudyBase.h"

namespace compress {

  CompressionStudyBase::CompressionStudyBase(){

    _verbose = false; 
  }
  
  void CompressionStudyBase::StudyCompression(const std::vector<short> &in,
					      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
					      const int pl)
  {
    
    return;
  }
  
}

#endif

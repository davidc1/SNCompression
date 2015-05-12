/**
 * \file StudyCompressionBufferSize.h
 *
 * \ingroup Analysis
 * 
 * \brief Class def header for a class StudyCompressionBufferSize
 *
 * @author David Caratelli
 */

/** \addtogroup Analysis

    @{*/

#ifndef STUDYCOMPRESSIONBUFFERSIZE_H
#define STUDYCOMPRESSIONBUFFERSIZE_H

#include "Analysis/ana_base.h"
#include "CompressionAlgoBase.h"
#include "LArUtil/Geometry.h"
#include "DataFormat/rawdigit.h"
#include "TH1D.h"
#include "TF1.h"
#include "TTree.h"
#include "TGraph.h"

namespace larlite {
  /**
     \class StudyCompressionBufferSize
     User custom analysis class made by SHELL_USER_NAME
   */
  class StudyCompressionBufferSize : public ana_base{
  
  public:

    /// Default constructor
    StudyCompressionBufferSize();

    /// Default destructor
    virtual ~StudyCompressionBufferSize(){};

    /** IMPLEMENT in StudyCompressionBufferSize.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in StudyCompressionBufferSize.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in StudyCompressionBufferSize.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    /// Study baseline effects due to compression
    void StudyBaseline(const std::pair< compress::tick, compress::tick> &range);

    /// Setup model for baseline fitting
    void PrepareLinearModel();

    /// Set Compression Algorithm
    void SetCompressAlgo(compress::CompressionAlgoBase* algo) { _compress_algo = algo; }

    protected:
    
    /// Compression Algorithm Object...performs compression
    compress::CompressionAlgoBase* _compress_algo;
    
    /// event_rawdigit: temporary holder for output WFs
    larlite::event_rawdigit _out_event_wf;

    /// holder for the # of ticks in the original waveforms
    double _inTicks;
    /// holder for the # of ticks in the compressed waveforms
    double _outTicks;

    // Histogram for per-event compression factor
    int    _evt;

    // Values for baseline fitting
    TTree *_base_tree;
    double _base_pre;
    double _base_post;
    double _slope_pre;
    double _slope_post;
    double _baseChan;
    double _varChan;
    int  _ch;
    int  _sygType;
    int  _plane;

    // Model for fitting
    TF1 *Model;


  };
}
#endif

/** @} */ // end of doxygen group 

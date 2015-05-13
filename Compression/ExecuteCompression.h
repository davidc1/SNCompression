/**
 * \file ExecuteCompression.h
 *
 * \ingroup Analysis
 * 
 * \brief Class def header for a class ExecuteCompression
 *
 * @author David Caratelli
 */

/** \addtogroup Analysis

    @{*/

#ifndef EXECUTECOMPRESSION_H
#define EXECUTECOMPRESSION_H

#include "Analysis/ana_base.h"
#include "CompressionAlgoBase.h"
#include "CompressionStudyBase.h"
#include "LArUtil/Geometry.h"
#include "DataFormat/rawdigit.h"
#include "TH1D.h"
#include <TStopwatch.h>

namespace larlite {
  /**
     \class ExecuteCompression
     User custom analysis class made by SHELL_USER_NAME
   */
  class ExecuteCompression : public ana_base{
  
  public:

    /// Default constructor
    ExecuteCompression();

    /// Default destructor
    virtual ~ExecuteCompression(){};

    /** IMPLEMENT in ExecuteCompression.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in ExecuteCompression.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in ExecuteCompression.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    /// Set Compression Algorithm
    void SetCompressAlgo(compress::CompressionAlgoBase* algo) { _compress_algo = algo; }

    /// Set Compression Study
    void SetCompressStudy(compress::CompressionStudyBase* study) { _compress_study = study; }

    /// Calculate compression: keep holders for number of ticks in original waveform and number of ticks in compressed waveforms
    void CalculateCompression(const std::vector<short> &beforeADCs,
			      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			      int pl);

    /// Decide here if to save output
    void SetSaveOutput(bool on) { _saveOutput = on; }

    protected:

    /// Now filll output WFs information into larlite data product so that we can write to output
    void SwapData(const larlite::rawdigit* tpc_data,
		  const std::vector<std::pair< compress::tick, compress::tick> > &ranges);
    
    /// Compression Algorithm Object...performs compression
    compress::CompressionAlgoBase* _compress_algo;

    /// Compression Study function -> do histograms...whatever
    compress::CompressionStudyBase* _compress_study;
    
    /// event_rawdigit: temporary holder for output WFs
    larlite::event_rawdigit _out_event_wf;

    // Boolean to decide whether to save output
    bool _saveOutput;

    /// holder for the # of ticks in the original waveforms
    double _inTicks;
    /// holder for the # of ticks in the compressed waveforms
    double _outTicks;

    // Histogram for per-event compression factor
    TTree *_compress_tree;
    double _compression;
    double _compressionU;
    double _compressionV;
    double _compressionY;
    int    _evt;
    // keep track of number of wires scanned per plane (to calculate compession)
    int _NplU, _NplV, _NplY;

    // timer to keep track of time-performance
    TStopwatch _evtwatch; // full event time
    TStopwatch _loopwatch;
    TStopwatch _watch;
    double _time_loop, _time_get, _time_algo, _time_study, _time_calc, _time_swap;


  };
}
#endif

/** @} */ // end of doxygen group 

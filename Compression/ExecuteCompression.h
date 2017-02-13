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

// Ana includes
#include "Analysis/ana_base.h"
// Compression includes
#include "CompressionAlgoBase.h"
#include "CompressionStudyBase.h"
#include "CompressionStudyIDEs.h"
#include "ViewCompression.h"
// LArUtil includes
#include "LArUtil/Geometry.h"
// DataFormat includes
#include "DataFormat/rawdigit.h"
#include "DataFormat/wire.h"
#include "DataFormat/simch.h"
// ROOT includes
#include "TH1D.h"
#include <TStopwatch.h>
#include <map>

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

    /// Set Compression Viewer
    void SetCompressViewer(compress::ViewCompression* viewer) { _compress_view = viewer; }

    /// Set IDE study algorithm
    void SetIDEStudy(compress::CompressionStudyIDEs* ide) { _ide_study = ide; }

    /// Calculate compression: keep holders for number of ticks in original waveform and number of ticks in compressed waveforms
    void CalculateCompression(const std::vector<short> &beforeADCs,
			      const std::vector<std::pair< compress::tick, compress::tick> > &ranges, int postHuff,
			      int pl, int ch);

    /// Decide here if to save output
    void SetSaveOutput(bool on) { _saveOutput = on; }

    /// function that calls the compression algo
    void ApplyCompression(const size_t i);

    // apply compression given channel number
    void ApplyCompressionChannel(const size_t ch);

    /// get number of waveforms saved
    int GetNumWFs() { return _event_wf->size(); }

    /// fill map for simch info
    void fillSimchMap(const larlite::event_simch* ev_simch);

    /// use simch info?
    void SetUseSimch(bool on) { _use_simch = on; }

    /// add channel range
    void addChannelRange(unsigned int min, unsigned int max) {
      std::pair<unsigned int, unsigned int> range;
      range = std::make_pair(min,max);
      _ch_range_v.push_back( range );
    }

  protected:

    /// Now filll output WFs information into larlite data product so that we can write to output
    void SwapData(const larlite::rawdigit* tpc_data,
		  const std::vector<std::pair< compress::tick, compress::tick> > &ranges);

    /// calculate Huffman compression (returns number of huffman words saved)
    int HuffmanCompression(const larlite::rawdigit* tpc_data,
			   const std::vector<std::pair< compress::tick, compress::tick> > &ranges);

    // check if chanels is in range
    bool isinrange(unsigned int ch);
    
    /// Compression Algorithm Object...performs compression
    compress::CompressionAlgoBase* _compress_algo;

    /// Compression Study function -> do histograms...whatever
    compress::CompressionStudyBase* _compress_study;

    /// compression viewer...shows waveform compression
    compress::ViewCompression* _compress_view;

    /// compression ide study
    compress::CompressionStudyIDEs* _ide_study;

    /// holder for input rawdigit
    larlite::event_rawdigit* _event_wf;
    
    /// event_wire: temporary holder for output WFs
    larlite::event_wire* _event_wire;

    /// event simch info
    larlite::event_simch* _event_simch;

    /// simch map
    std::map<unsigned int,std::vector<std::pair<unsigned short, double> > >_simchMap;

    // range of channels to be used
    std::vector<std::pair<unsigned int,unsigned int> > _ch_range_v;

    // Boolean to decide whether to save output
    bool _saveOutput;

    // boolean to decide if to read/use simch info
    bool _use_simch;

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
    // Tree for channel-by-channel compression
    TTree *_compress_ch;
    double _ch_compression;
    int    _ch;
    int    _pl;
    // keep track of number of wires scanned per plane (to calculate compession)
    int _NplU, _NplV, _NplY;
	//Anya variables:
    double _compression_huff;
    double _compressionU_huff;
    double _compressionV_huff;
    double _compressionY_huff;
    double _ch_compression_huff;
    double _postHuffmanwords;
    // timer to keep track of time-performance
    TStopwatch _evtwatch; // full event time
    TStopwatch _loopwatch;
    TStopwatch _watch;
    double _time_loop, _time_get, _time_algo, _time_study, _time_calc, _time_swap, _time_ide, _time_read;

    // vector linking channel number to entry in vector of raw-digits
    std::vector<int> _channel_to_index_map;

  };
}
#endif

/** @} */ // end of doxygen group 

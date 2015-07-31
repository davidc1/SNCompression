/**
 * \file MCStudyCompression.h
 *
 * \ingroup Analysis
 * 
 * \brief Class def header for a class MCStudyCompression
 *
 * @author David Caratelli
 */

/** \addtogroup Analysis

    @{*/

#ifndef MCSTUDYCOMPRESSION_H
#define MCSTUDYCOMPRESSION_H

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
#include "DataFormat/simch.h"
#include "DataFormat/mcpart.h"
#include "DataFormat/mcshower.h"
// ROOT includes
#include "TH1D.h"
#include <map>
#include <algorithm> //std::find

namespace larlite {
  /**
     \class MCStudyCompression
     User custom analysis class made by SHELL_USER_NAME
   */
  class MCStudyCompression : public ana_base{
  
  public:

    /// Default constructor
    MCStudyCompression();

    /// Default destructor
    virtual ~MCStudyCompression(){};

    /** IMPLEMENT in MCStudyCompression.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in MCStudyCompression.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in MCStudyCompression.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    /// Set Compression Algorithm
    void SetCompressAlgo(compress::CompressionAlgoBase* algo) { _compress_algo = algo; }

    /// Calculate compression: keep holders for number of ticks in original waveform and number of ticks in compressed waveforms
    void CalculateCompression(const std::vector<short> &beforeADCs,
			      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			      int pl);

    /// function that calls the compression algo
    void ApplyCompression(const size_t i);

    /// get number of waveforms saved
    int GetNumWFs() { return _event_wf->size(); }

    /// fill map for simch info
    void fillSimchMap(const larlite::event_simch* ev_simch);

    /// set verbosity
    void setVerbose(bool on) { _verbose = on; }


  protected:

    /// debug boolean
    bool _verbose;

    void calculateParticleEfficiency(int trkID, double& E, double& Eout,
				     std::vector<double>& Eplane,
				     std::vector<double>& EplaneOut,
				     unsigned int& tickMax, unsigned int& tickMin);

    // calcualte the efficiency for a given simch
    void SimchEfficiency(const simch& simchan,
			 const int& trkID,
			 const compress::tick& WFbegin,
			 const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
			 double& Edep, double& EdepOut,
			 std::vector<double>& EdepPlane,
			 std::vector<double>& EdepPlaneOut,
			 unsigned int& tickMax, unsigned int& tickMin);
    
    // is a tick in the output?
    bool isTickInOutput(const unsigned short& TDC,
			const compress::tick& WFbegin,
			const std::vector<std::pair<compress::tick,compress::tick> >& ranges);

    /// fill MCPart TrackID -> position in _event_mcpart map
    void fillMCPartMap();

    /// fill TrkID -> vector of position of associated simchans
    void TrkIDtoSimchMap();

    /// Compression Algorithm Object...performs compression
    compress::CompressionAlgoBase* _compress_algo;

    /// holder for input rawdigit
    larlite::event_rawdigit* _event_wf;
    
    /// event_rawdigit: temporary holder for output WFs
    larlite::event_rawdigit _out_event_wf;

    /// event simch info
    larlite::event_simch* _event_simch;

    /// event mcpart info
    larlite::event_mcpart* _event_mcpart;

    /// event mcshower info
    larlite::event_mcshower* _event_mcshower;

    /// simch map (LAr Channel -> simchannels associated [TDC,energy]
    std::map<unsigned int,std::vector<std::pair<unsigned short, double> > >_simchMap;
    /// map connecting TrackID to vector of SimChannel positions
    std::map<int,std::vector<size_t> > _simchTrkIDMap;
    /// waveform map (linking channel to position in _event_wf
    std::map<unsigned int,size_t> _inWfMap;
    /// waveform map (linking channel to output range)
    std::map<unsigned int,std::vector<std::pair< compress::tick, compress::tick> > > _outWfMap;
    /// map connecting channel to iterator to begin of EF
    std::map<unsigned int,std::vector<short>::const_iterator> _inBeginMap;
    /// map connecting mcpart trackID and position in _event_mcpart
    std::map<int,size_t> _mcPartMap;

    /// holder for the # of ticks in the original waveforms
    double _inTicks;
    /// holder for the # of ticks in the compressed waveforms
    double _outTicks;

    /// number of ticks to consider (to fit 3*64 tick segments)
    int _wfLen;

    // Histogram for per-event compression factor
    TTree *_compress_tree;
    double _compression;
    double _compressionU;
    double _compressionV;
    double _compressionY;
    int    _evt;
    
    // histogram for per-mcshower efficiency
    TTree *_mcpart_tree;
    int _trkID;
    int _PDG;
    double _E;
    double _EdepU, _EdepUout;
    double _EdepV, _EdepVout;
    double _EdepY, _EdepYout;
    double _EdepT, _EdepTout;
    unsigned int _tickMax, _tickMin;

    // keep track of number of wires scanned per plane (to calculate compession)
    int _NplU, _NplV, _NplY;

  };
}
#endif

/** @} */ // end of doxygen group 

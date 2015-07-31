#ifndef MCSTUDYCOMPRESSION_CXX
#define MCSTUDYCOMPRESSION_CXX

#include "MCStudyCompression.h"

namespace larlite {

  MCStudyCompression::MCStudyCompression()
    : _compress_algo(nullptr)
    , _event_wf(nullptr)
    , _event_simch(nullptr)
    , _event_mcpart(nullptr)
    , _event_mcshower(nullptr)
    , _compress_tree(nullptr)
    , _mcpart_tree(nullptr)
  {
    _fout = 0;
    _verbose = false;
  }

  bool MCStudyCompression::initialize() {
    
    // Initalize Histogram that tracks compression factor
    if (!_compress_tree) { _compress_tree = new TTree("_compress_tree","Compression Info Tree"); }
    _compress_tree->Branch("_evt",&_evt,"evt/I");
    _compress_tree->Branch("_compression",&_compression,"compression/D");
    _compress_tree->Branch("_compressionU",&_compressionU,"compressionU/D");
    _compress_tree->Branch("_compressionV",&_compressionV,"compressionV/D");
    _compress_tree->Branch("_compressionY",&_compressionY,"compressionY/D");

    if (!_mcpart_tree) { _mcpart_tree = new TTree("_mcpart_tree","MCPart Efficiency Tree"); }
    _mcpart_tree->Branch("_evt",&_evt,"evt/I");
    _mcpart_tree->Branch("_trkID",&_trkID,"trkID/I");
    _mcpart_tree->Branch("_PDG",&_PDG,"PDG/I");
    _mcpart_tree->Branch("_E",&_E,"E/D");
    _mcpart_tree->Branch("_EdepT",&_EdepT,"EdepT/D");
    _mcpart_tree->Branch("_EdepU",&_EdepU,"EdepU/D");
    _mcpart_tree->Branch("_EdepV",&_EdepV,"EdepV/D");
    _mcpart_tree->Branch("_EdepY",&_EdepY,"EdepY/D");
    _mcpart_tree->Branch("_EdepTout",&_EdepTout,"EdepTout/D");
    _mcpart_tree->Branch("_EdepUout",&_EdepUout,"EdepUout/D");
    _mcpart_tree->Branch("_EdepVout",&_EdepVout,"EdepVout/D");
    _mcpart_tree->Branch("_EdepYout",&_EdepYout,"EdepYout/D");
    _mcpart_tree->Branch("_tickMax",&_tickMax,"tickMax/I");
    _mcpart_tree->Branch("_tickMin",&_tickMin,"tickMin/I");
    _compressionU = 0;
    _compressionV = 0;
    _compressionY = 0;
    _compression  = 0;
    _NplU = _NplV = _NplY = 0;

    _evt = 0;

    return true;
  }
  
  bool MCStudyCompression::analyze(storage_manager* storage) {

    _evt += 1;

    // clear the input/output waveform maps
    _inWfMap.clear();
    _outWfMap.clear();
    _inBeginMap.clear();

    // If no compression algorithm has been defined, skip
    if ( _compress_algo == 0 ){
      print(msg::kERROR,__FUNCTION__,"Compression Algorithm Not Set! Exiting");
      return false;
    }

    // Otherwise Get RawDigits and execute compression
    _event_wf = storage->get_data<event_rawdigit>("daq");
    // If raw_digits object is empty -> exit
    if(!_event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }

    // get mcparts
    _event_mcpart = storage->get_data<event_mcpart>("largeant");
    if(!_event_mcpart) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated mcparticles!");
      return false;
    }
    // fill trackID -> position in _event_mcpart map
    fillMCPartMap();

    // get mcparts
    _event_simch = storage->get_data<event_simch>("largeant");
    if(!_event_simch) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated simchannels!");
      return false;
    }
    // fill trackID -> position in _event_simch of associated simchannels
    TrkIDtoSimchMap();

    // get mcshowers
    _event_mcshower = storage->get_data<event_mcshower>("mcreco");
    if(!_event_mcshower) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated MCShowers!");
      return false;
    }

    // clear place-holder for new, compressed, waveforms
    _out_event_wf.clear();

    // reset variables that hold compression factor
    _inTicks  = 0;
    _outTicks = 0;

    for (size_t i=0; i< _event_wf->size(); i++){
      //get tpc_data
      _inWfMap[_event_wf->at(i).Channel()] = i;
      ApplyCompression(i);
    }//for all waveforms
    if (_verbose) { std::cout << "Compression applied to all channels!" << std::endl; }

    // for each mcshower, get the associated IDEs and see what
    // fraction of their energy has been saved in output
    if (_verbose) { std::cout << "found " << _event_mcshower->size() << " mcshowers" << std::endl; }
    for (size_t j=0; j < _event_mcshower->size(); j++){
      _PDG   = _event_mcshower->at(j).PdgCode();
      _trkID = _event_mcshower->at(j).TrackID();
      _E     = _event_mcshower->at(j).DetProfile().E();
      _EdepT  = _EdepTout = 0;
      _tickMax = 0;
      _tickMin = 9600;
      std::vector<double> Eplane = {0,0,0};
      std::vector<double> EplaneOut = {0,0,0};
      if (_verbose) { std::cout << "Calculate Eff for MCShwoer ID: " << _trkID << "\tPDG: " << _PDG << "\tEdep: " << _E << std::endl; }
      calculateParticleEfficiency(_trkID,_EdepT,_EdepTout,Eplane,EplaneOut,_tickMax,_tickMin);
      _EdepU = Eplane[0];
      _EdepV = Eplane[1];
      _EdepY = Eplane[2];
      _EdepUout = EplaneOut[0];
      _EdepVout = EplaneOut[1];
      _EdepYout = EplaneOut[2];

      if (_verbose) { std::cout << "Eff found to be: Edep: " << _EdepT << "\tEdepOut: " << _EdepTout << std::endl; }
      // fill tree
      _mcpart_tree->Fill();
    }
    
    //std::cout << "U planes: " << _NplU << "\tV: " << _NplV << "\tY: " << _NplY << std::endl;
    _compressionU /= 2399.;//_NplU;
    _compressionV /= 2399.;//_NplV;
    _compressionY /= 3456.;//_NplY;
    _compression  /= (2399.+2399.+3456.);//(_NplU+_NplV+_NplY);
    _compress_tree->Fill();
    _NplU = _NplV = _NplY = 0;
    _compressionU = _compressionV = _compressionY = 0;
    
    return true;
  }

  bool MCStudyCompression::finalize() {

    if (_compress_algo)
      _compress_algo->EndProcess(_fout);

    _compress_tree->Write();
    _mcpart_tree->Write();

    return true;
  }


  // function where compression is applied on a single wf
  void MCStudyCompression::ApplyCompression(const size_t i)
  {

    const larlite::rawdigit* rawwf = &(_event_wf->at(i));

      //Check for empty waveforms!
    if(rawwf->ADCs().size()<1){
      print(msg::kERROR,__FUNCTION__,
	    Form("Found 0-length waveform: Ch. %d",rawwf->Channel()));
      return;
    }//if wf size < 1

    // Figure out channel's plane:
    // used because different planes will have different "buffers"
    UInt_t ch = rawwf->Channel();
    int pl = larutil::Geometry::GetME()->ChannelToPlane(ch);
    
    // finally, apply compression:
    // *-------------------------*
    // 1) Convert tpc_data object to just the vector of shorts which make up the ADC ticks
    const std::vector<short> ADCwaveformL = rawwf->ADCs();
    // cut size so that 3 blocks fit perfectly
    int nblocks = ADCwaveformL.size()/(3*64);
    std::vector<short>::const_iterator first = ADCwaveformL.begin();
    std::vector<short>::const_iterator last  = ADCwaveformL.begin()+(3*64*nblocks);
    _wfLen = 3*64*nblocks;
    std::vector<short> ADCwaveform(first,last);
    // 2) Now apply the compression algorithm. _compress_algo is an instance of CompressionAlgoBase
    _compress_algo->ApplyCompression(ADCwaveform,pl,ch);
    // 3) Retrieve output ranges saved
    auto const& ranges = _compress_algo->GetOutputRanges();
    // fill output map for waveforms
    _inBeginMap[ch] = _compress_algo->GetInputBegin();
    _outWfMap[ch] = ranges;
    // 9) Calculate compression factor [ for now Ticks After / Ticks Before ]
    CalculateCompression(ADCwaveform, ranges, pl);
    // 10) clear _InWF and _OutWF from compression algo object -> resetting algorithm for next time it is called
    _compress_algo->Reset();
    
    return;
  }

  // given an MCParticle's associated IDEs, find what fraction of them have
  // been saved by compression
  void MCStudyCompression::calculateParticleEfficiency(int trkID, double& E, double& Eout,
						       std::vector<double>& Eplane,
						       std::vector<double>& EplaneOut,
						       unsigned int& tickMax,
						       unsigned int& tickMin){


    // get simchannel positions associated with this trackID
    std::vector<size_t> pos = _simchTrkIDMap[trkID];
    //if (_verbose) { std::cout << "simchannels associated with trkID: " << trkID << " = " << pos.size() << std::endl; }
    // loop over simchannels that have E deposited by this trackID
    for (auto const& s : pos){
      
      // get simch
      auto const& simchan = _event_simch->at(s);
      // find WF channel associated with this simch
      auto ch = simchan.Channel();
      if (_verbose) { std::cout << "\tChannel: " << ch << std::endl; }
      // get iterator to beginning of input WF
      auto const& begin = _inBeginMap[ch];//_event_wf->at(_inWfMap[ch]).ADCs().begin();
      // get output ranges
      auto const& ranges = _outWfMap[ch];
      // calculate efficiency
      SimchEfficiency(simchan,trkID,begin,ranges,E,Eout,Eplane,EplaneOut,tickMax,tickMin);
    }
    // now recursively repeat for all children
    auto const& daughters = _event_mcpart->at(_mcPartMap[trkID]).Daughters();
    std::set<int>::const_iterator dit;
    for (dit = daughters.begin(); dit != daughters.end(); dit++){
      int dTrkID = *dit;
      auto const daught = _event_mcpart->at(_mcPartMap[dTrkID]);
      if (dTrkID != trkID)
	calculateParticleEfficiency(dTrkID,E,Eout,Eplane,EplaneOut,tickMax,tickMin);
    }

    return;
  }


  // given a simch: calculate the energy deposited and that stored in the output waveform
  void MCStudyCompression::SimchEfficiency(const simch& simchan,
					   const int& trkID,
					   const compress::tick& WFbegin,
					   const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
					   double& Edep, double& EdepOut,
					   std::vector<double>& EdepPlane,
					   std::vector<double>& EdepPlaneOut,
					   unsigned int& tickMax,
					   unsigned int& tickMin)
  {

    // get plane
    UInt_t ch = simchan.Channel();
    int pl = larutil::Geometry::GetME()->ChannelToPlane(ch);
    // get map of TDC to IDEs
    auto const& ideMap = simchan.TDCIDEMap();
    // loop over map elements
    std::map<unsigned short, std::vector<larlite::ide> >::const_iterator it;
    for (it = ideMap.begin(); it != ideMap.end(); it++){
      // get time-tick
      auto const& ttick = it->first;
      if (it->first > _wfLen) //{ std::cout << it->first << std::endl; }
	continue;
      if (ttick > tickMax) { tickMax = ttick; }
      if (ttick < tickMin) { tickMin = ttick; }
      // get ides @ this tick
      auto const& ides = it->second;
      for (auto const& ide : ides){
	// if not the trackID we are interested in
	if (ide.trackID != trkID)
	  continue;
	// otherwise, get the energy, and check if the TDC has been saved
	if (_verbose) { std::cout << "\t\tIDE trkID: " << ide.trackID << "\tE: " << ide.energy << "\tTick: " << it->first << std::endl; }
	Edep += ide.energy;
	EdepPlane[pl] += ide.energy;
	if (isTickInOutput(it->first,WFbegin,ranges) == true){
	  EdepOut += ide.energy;
	  EdepPlaneOut[pl] += ide.energy;
	}
	
      }// for all IDEs
    }//for all ides associated with a time-tick

    return;
  }
	
  bool MCStudyCompression::isTickInOutput(const unsigned short& TDC,
					  const compress::tick& WFbegin,
					  const std::vector<std::pair<compress::tick,compress::tick> >& ranges){


    //std::cout << "TDC: " << TDC << std::endl;
    // ok, start looping
    for (auto const& range: ranges){
      //std::cout << "this range: [" << std::distance(WFbegin,range.first)
      //	<< ", " << std::distance(WFbegin,range.second) << "]" << std::endl;
      if ( (TDC > std::distance(WFbegin,range.first)) and
	   (TDC < std::distance(WFbegin,range.second)) )
	return true;
    }

    return false;
  }


  void MCStudyCompression::CalculateCompression(const std::vector<short> &beforeADCs,
						const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
						int pl){
    
    double inTicks = beforeADCs.size();
    double outTicks = 0;
    
    for (size_t n=0; n < ranges.size(); n++)
      outTicks += (ranges[n].second-ranges[n].first);

    if (pl==0){
      _compressionU += outTicks/inTicks;
      _NplU += 1;
    }
    else if (pl==1){
      _compressionV += outTicks/inTicks;
      _NplV += 1;
    }
    else if (pl==2){
      _compressionY += outTicks/inTicks;
      _NplY += 1;
    }
    else
      std::cout << "What plane? Error?" << std::endl;
    
    _compression += outTicks/inTicks;

    return;
  }


  /// Fill TrackID -> simchannels associated map
  // links to position of simch in simch vector
  void MCStudyCompression::TrkIDtoSimchMap(){

    _simchTrkIDMap.clear();

    for (size_t s=0; s < _event_simch->size(); s++){
      auto const simch = _event_simch->at(s);
      // get map of TDC -> vector<ides> for this simch object
      const std::map<unsigned short, std::vector<larlite::ide> > ideMap = simch.TDCIDEMap();
      std::map<unsigned short, std::vector<larlite::ide> >::const_iterator it;
      for (it = ideMap.begin(); it != ideMap.end(); it++){
	// loop over all ides:
	auto const& ides = it->second;
	for (auto const& ide : ides){
	  // get trackID
	  int trkID = ide.trackID;
	  // does this track exist in map? if no -> create new entry
	  if (_simchTrkIDMap.find(trkID) == _simchTrkIDMap.end()){
	    std::vector<size_t> pos = {s};
	    _simchTrkIDMap[trkID] = pos;
	  }
	  else{
	    // trkID already stored
	    // has this channel been entered already? if so remove
	    std::vector<size_t> simchans = _simchTrkIDMap[trkID];
	    // add only if not yet listed
	    std::vector<size_t>::iterator sit;
	    sit = std::find(simchans.begin(),simchans.end(),s);
	    if (sit == simchans.end())
	      _simchTrkIDMap[trkID].push_back(s);
	  }
	}// for all IDEs
      }// for all IDEs in this simch
    }// for all simchs

    return;
  }

  // Fill Simch Map to get simchannels associated with a channel
  void MCStudyCompression::fillSimchMap(const larlite::event_simch* ev_simch)
  {

    _simchMap.clear();
    //    _simchMap.reserve(_event_wf->size());
    for (size_t i=0; i < ev_simch->size(); i++){
      auto const simch = ev_simch->at(i);
      // get map of TDC -> vector<ides> for this simch object
      const std::map<unsigned short, std::vector<larlite::ide> > ideMap = simch.TDCIDEMap();
      // create a vector which connects TCD to energy of ide
      std::vector<std::pair<unsigned short,double> > _ide_v;
      std::map<unsigned short, std::vector<larlite::ide> >::const_iterator it;
      for (it = ideMap.begin(); it != ideMap.end(); it++){
	double Etot = 0; // total energy at this tick
	auto const idevec = it->second;
	for (auto const& ide : idevec)
	  Etot += ide.energy;
	_ide_v.push_back(std::make_pair(it->first,Etot));
      }
      _simchMap[simch.Channel()] = _ide_v;
    }// for all simchannel objects

    return;
  }

  
  void MCStudyCompression::fillMCPartMap(){

    _mcPartMap.clear();
    for (size_t p=0; p < _event_mcpart->size(); p++)
      _mcPartMap[_event_mcpart->at(p).TrackId()] = p;

    return;
  }

}
#endif

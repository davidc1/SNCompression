#ifndef EXECUTECOMPRESSION_CXX
#define EXECUTECOMPRESSION_CXX

#include "ExecuteCompression.h"

namespace larlite {

  ExecuteCompression::ExecuteCompression()
    : _compress_algo(nullptr)
    , _compress_study(nullptr)
    , _compress_view(nullptr)
    , _ide_study(nullptr)
    , _compress_tree(nullptr)
    , _compress_ch(nullptr)
  {
    _fout = 0;
    _saveOutput = false;    
    _use_simch  = false;
    _ch_range_v.clear();
  }

  bool ExecuteCompression::initialize() {
    
    // Initalize Histogram that tracks compression factor
    if (!_compress_tree) { _compress_tree = new TTree("_compress_tree","Compression Info Tree"); }
    _compress_tree->Branch("_evt",&_evt,"evt/I");
    _compress_tree->Branch("_compression",&_compression,"compression/D");
    _compress_tree->Branch("_compressionU",&_compressionU,"compressionU/D");
    _compress_tree->Branch("_compressionV",&_compressionV,"compressionV/D");
    _compress_tree->Branch("_compressionY",&_compressionY,"compressionY/D");
    //Anya variables: 
 
    _compress_tree->Branch("_compression_huff",&_compression_huff,"compression_huff/D");
    _compress_tree->Branch("_compressionU_huff",&_compressionU_huff,"compressionU_huff/D");
    _compress_tree->Branch("_compressionV_huff",&_compressionV_huff,"compressionV_huff/D");
    _compress_tree->Branch("_compressionY_huff",&_compressionY_huff,"compressionY_huff/D");

    if (_compress_ch) delete _compress_ch;
    _compress_ch = new TTree("_compress_ch","Channel-by-Channel Compress. Tree");
    _compress_ch->Branch("_ch_compression",&_ch_compression,"ch_compression/D");
    _compress_ch->Branch("_ch_compression_huff",&_ch_compression_huff,"ch_compression_huff/D");//Anya variable
    _compress_ch->Branch("_ch",&_ch,"ch/I");
    _compress_ch->Branch("_pl",&_pl,"pl/I");

    _compressionU = 0;
    _compressionV = 0;
    _compressionY = 0;
    _compression  = 0;
    _NplU = _NplV = _NplY = 0;
    //Anya variables
    _compressionU_huff = 0;
    _compressionV_huff = 0;
    _compressionY_huff = 0;
    _compression_huff  = 0;


    _evt = 0;

    _time_loop = _time_get = _time_algo = _time_study = _time_calc = _time_swap = _time_ide = _time_read = 0;

    _evtwatch.Start();

    return true;
  }
  
  bool ExecuteCompression::analyze(storage_manager* storage) {

    _evt += 1;

    // If no compression algorithm has been defined, skip
    if ( _compress_algo == 0 ){
      print(msg::kERROR,__FUNCTION__,"Compression Algorithm Not Set! Exiting");
      return false;
    }

    _watch.Start();
    
    // Get RawDigits and execute compression
    _event_wf   = storage->get_data<event_rawdigit>("daq");

    // Load output wire data if to be saved
    _event_wire = storage->get_data<event_wire>("caldata");

    storage->set_id(storage->run_id(), storage->subrun_id(), storage->event_id());
    
    // If raw_digits object is empty -> exit
    if(!_event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }

    // get simch
    if (_use_simch){
      _event_simch = storage->get_data<event_simch>("largeant");
      // make a map: channel -> associated simch
      fillSimchMap(_event_simch);
    }

    //  map channel number to entry index
    _channel_to_index_map.clear();
    _channel_to_index_map = std::vector<int>( ::larutil::Geometry::GetME()->Nchannels(), -1);
    for (size_t i=0; i < _event_wf->size(); i++)
      _channel_to_index_map.at( _event_wf->at(i).Channel() ) = (int)i;

    _time_read = _watch.RealTime();

    // clear place-holder for new, compressed, waveforms
    //_out_event_wf.clear();

    // reset variables that hold compression factor
    _inTicks  = 0;
    _outTicks = 0;
    //_postHuffTicks = 0; //Anya variable

    // if we want to use the viewer -> skip this
    if (_compress_view)
      return true;
    
    // Loop over all waveforms
    _loopwatch.Start();
    for (size_t i=0; i<_event_wf->size(); i++){
      //get tpc_data
      ApplyCompression(i);
    }//for all waveforms
    _time_loop += _loopwatch.RealTime();
    
    //std::cout << "U planes: " << _NplU << "\tV: " << _NplV << "\tY: " << _NplY << std::endl;
    _compressionU /= 2399.;//_NplU;
    _compressionV /= 2399.;//_NplV;
    _compressionY /= 3456.;//_NplY;
    _compression  /= (2399.+2399.+3456.);//(_NplU+_NplV+_NplY);
    _compress_tree->Fill();
    _NplU = _NplV = _NplY = 0;
    _compressionU = _compressionV = _compressionY = 0;

    /*
    //now take new WFs and place in event_wf vector
    if (_saveOutput){
      _event_wf->clear();
      for (size_t m=0; m < _out_event_wf.size(); m++)
	_event_wf->push_back(_out_event_wf.at(m));
    }
    */

    return true;
  }

  bool ExecuteCompression::finalize() {

    if (_fout) _fout->cd();

    double tottime = _evtwatch.RealTime();
    
    std::cout.precision(3);
    //std::scientific;

    std::cout << "  \033[95m<<Tot Time>>\033[00m  : " << tottime << " [s] ... or "
	      << tottime/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Read Time>>\033[00m : " << _time_read << " [s] ... or "
	      << _time_read/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Loop Time>>\033[00m : " << _time_loop << " [s] ... or "
	      << _time_loop/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Get Time>>\033[00m  : ... " << _time_get << " [s] ... or "
	      << _time_get/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Algo Time>>\033[00m : ... " << _time_algo << " [s] ... or "
	      << _time_algo/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Study Time>>\033[00m: ... " << _time_study << " [s] ... or "
	      << _time_study/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<IDEs Time>>\033[00m : ... " << _time_ide << " [s] ... or "
	      << _time_ide/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Calc Time>>\033[00m : ... " << _time_calc << " [s] ... or "
	      << _time_calc/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Swap Time>>\033[00m : ... " << _time_swap << " [s] ... or "
	      << _time_swap/_evt << " [s/evt]" << std::endl;

    if (_compress_algo)
      _compress_algo->EndProcess(_fout);
    if (_compress_study)
      _compress_study->EndProcess(_fout);
    if (_ide_study)
      _ide_study->EndProcess(_fout);

    _compress_tree->Write();

    _compress_ch->Write();

    return true;
  }

  // apply compression to a specific channel
  void ExecuteCompression::ApplyCompressionChannel(const size_t ch){

    if (_channel_to_index_map.size() <= ch) return;
    if ( _channel_to_index_map[ch] == -1 ) return;

    ApplyCompression( _channel_to_index_map[ch] );
  }
  
  
  // function where compression is applied on a single wf
  void ExecuteCompression::ApplyCompression(const size_t i)
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

    // is the channel in the channel range?
    if (!isinrange(ch))
      return;
    
    // finally, apply compression:
    // *-------------------------*
    // 1) Convert tpc_data object to just the vector of shorts which make up the ADC ticks
    _watch.Start();
    const std::vector<short> ADCwaveformL = rawwf->ADCs();
    // cut size so that 3 blocks fit perfectly
    int nblocks = ADCwaveformL.size()/(3*64);
    std::vector<short>::const_iterator first = ADCwaveformL.begin();
    std::vector<short>::const_iterator last  = ADCwaveformL.begin()+(3*64*nblocks);
    std::vector<short> ADCwaveform(first,last);
    _time_get += _watch.RealTime();
    // 2) Now apply the compression algorithm. _compress_algo is an instance of CompressionAlgoBase
    _watch.Start();
    _compress_algo->ApplyCompression(ADCwaveform,pl,ch);
    _time_algo += _watch.RealTime();
    // 3) Retrieve output ranges saved
    auto const& ranges = _compress_algo->GetOutputRanges();
    int postHuffwords = _compress_algo->GetHuffman();
    // 6) Study the Compression results for this channel
    _watch.Start();
    if (_compress_study)
      _compress_study->StudyCompression(ADCwaveform, ranges, pl);
    _time_study += _watch.RealTime();
    // 7) View compression if viewer is set
    if (_compress_view){
      if (_simchMap.find(ch) != _simchMap.end())
	_compress_view->FillIDEs(_simchMap[ch],_evt,ch,pl,
				 std::distance(_compress_algo->GetInputBegin(),_compress_algo->GetInputEnd()));
      else
	_compress_view->ResetIDEs(_evt,ch,pl,
				  std::distance(_compress_algo->GetInputBegin(),_compress_algo->GetInputEnd()));
      _compress_view->FillHistograms(std::make_pair(_compress_algo->GetInputBegin(),_compress_algo->GetInputEnd()),
				     ranges,_evt,ch,pl);
      _compress_view->FillBaseVarHistos(_compress_algo->GetBaselines(),_compress_algo->GetVariances(),_evt,ch,pl);
    }
    // 8) study IDEs if algorithm was selected
    if ( (_simchMap.find(ch) != _simchMap.end()) and (_ide_study) ){    
      _watch.Start();
      _ide_study->StudyCompression(_simchMap[ch],
				   std::make_pair(_compress_algo->GetInputBegin(),_compress_algo->GetInputEnd()),
				   ranges,pl,ch,_evt);
      _time_ide += _watch.RealTime();
    }
    // 9) Calculate compression factor [ for now Ticks After / Ticks Before ]
    _watch.Start();
    postHuffwords = HuffmanCompression(rawwf,ranges);
    CalculateCompression(ADCwaveform, ranges, postHuffwords, pl, ch);
    _time_calc += _watch.RealTime();
    // 10) clear _InWF and _OutWF from compression algo object -> resetting algorithm for next time it is called
    // 12) Replace .root data file *event_wf* with new waveforms
    _watch.Start();
    if (_saveOutput)
      SwapData(rawwf, ranges);
    _time_swap += _watch.RealTime();
    
    return;
  }

  void ExecuteCompression::SwapData(const larlite::rawdigit *tpc_data,
				    const std::vector<std::pair< compress::tick, compress::tick> > &ranges)
  {

    // In this function we are taking the old larlite::rawdigit object and replacing it with
    // new larlite::rawdigit objects, one per output waveform coming from compression
    // Most of the information for this object (channel number, wire plane, etc) stays the same
    // What changes is the actual list of ADC counts, and the start time of the vector.

    UInt_t chan = tpc_data->Channel();

    larlite::sparse_vector<float> wf_ROIs;

    //loop over new waveforms created
    for (size_t n=0; n < ranges.size(); n++){
      // prepare output waveform
      compress::tick t;
      float first_tick = (float)*(ranges[n].first);
      std::vector<float> out;
      for (t = ranges[n].first; t < ranges[n].second; t++)
	out.push_back( (float)*t - first_tick );

      unsigned int start_tick = ranges[n].first - _compress_algo->GetInputBegin();

      wf_ROIs.add_range( start_tick, std::move(out) );

      /*
      larlite::rawdigit new_tpc_data( chan, out.size(), out, larlite::raw::kNone);
      new_tpc_data.SetPedestal(ranges[n].first-_compress_algo->GetInputBegin());
      _out_event_wf.push_back(new_tpc_data);
      */
    }// for all saved ROIs

    larlite::wire thiswire(std::move(wf_ROIs), chan, larutil::Geometry::GetME()->View(chan) );

    _event_wire->emplace_back( thiswire );
    
    return;
  }


  int ExecuteCompression::HuffmanCompression(const larlite::rawdigit *tpc_data,
					     const std::vector<std::pair< compress::tick, compress::tick> > &ranges)
  {

    int HuffWordsTOTAL;
    
    // calculate the Huffman number of words for this channel
    
    UInt_t chan = tpc_data->Channel();
    
    //loop over new waveforms created
    for (size_t n=0; n < ranges.size(); n++){
      // prepare output waveform
      compress::tick t;
      float first_tick = (float)*(ranges[n].first);
      std::vector<float> out;
      for (t = ranges[n].first; t < ranges[n].second; t++)
	out.push_back( (float)*t - first_tick );

      // HERE COMPUTE HUFFMAN ALGORITHM on "out" VECTOR
      // AND ADD NUMBER OF WORDS TO "HuffWordsTOTAL" COUNTER
      
    }// for all saved ROIs
    
    return HuffWordsTOTAL;
  }

  
  void ExecuteCompression::CalculateCompression(const std::vector<short> &beforeADCs,
						const std::vector<std::pair< compress::tick, compress::tick> > &ranges,int postHuff,
						int pl, int ch){
    
    double inTicks = beforeADCs.size();
    double outTicks = 0;
    const int wordsize = 16;    

    for (size_t n=0; n < ranges.size(); n++)
      outTicks += (ranges[n].second-ranges[n].first);

    if (pl==0){
      _compressionU += outTicks/inTicks;
      _compressionU_huff += (postHuff * wordsize)/inTicks;//Anya variable
      _NplU += 1;
    }
    else if (pl==1){
      _compressionV += outTicks/inTicks;
      _compressionV_huff += (postHuff * wordsize)/inTicks;//Anya variable
      _NplV += 1;
    }
    else if (pl==2){
      _compressionY += outTicks/inTicks;
      _compressionY_huff += (postHuff * wordsize)/inTicks;//Anya variable
      _NplY += 1;
    }
    else
      std::cout << "What plane? Error?" << std::endl;
    
    _ch_compression = outTicks/inTicks;

    _compression += outTicks/inTicks;

    //Anya variables:
    _ch_compression_huff = (postHuff * wordsize)/inTicks;
    _compression_huff += (postHuff * wordsize)/inTicks;

    _ch = ch;
    _pl = pl;
    _compress_ch->Fill();

    return;
  }


  // Fill Simch Map to get simchannels associated with a channel
  void ExecuteCompression::fillSimchMap(const larlite::event_simch* ev_simch)
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

  
  // check if channel is in range
  bool ExecuteCompression::isinrange(unsigned int ch){

    if (_ch_range_v.size() == 0){
      return true;
    }

    bool found = false;

    for (auto const& range : _ch_range_v){
      if ( (ch >= range.first) && (ch <= range.second) ){
	found = true;
	break;
      }// if in range
    }// for all ranges

    return found;
  }

}
#endif

#ifndef EXECUTECOMPRESSION_CXX
#define EXECUTECOMPRESSION_CXX

#include "ExecuteCompression.h"

namespace larlite {

  bool ExecuteCompression::initialize() {
    
    // Initalize Histogram that tracks compression factor
    if (!_compress_tree) { _compress_tree = new TTree("_compress_tree","Compression Info Tree"); }
    _compress_tree->Branch("_evt",&_evt,"evt/I");
    _compress_tree->Branch("_compression",&_compression,"compression/D");
    _compress_tree->Branch("_compressionU",&_compressionU,"compressionU/D");
    _compress_tree->Branch("_compressionV",&_compressionV,"compressionV/D");
    _compress_tree->Branch("_compressionY",&_compressionY,"compressionY/D");

    _compressionU = 0;
    _compressionV = 0;
    _compressionY = 0;
    _compression  = 0;
    _NplU = _NplV = _NplY = 0;

    _evt = 0;

    _time_loop = _time_get = _time_algo = _time_study = _time_calc = _time_swap = 0;

    _evtwatch.Start();

    return true;
  }
  
  bool ExecuteCompression::analyze(storage_manager* storage) {

    // If no compression algorithm has been defined, skip
    if ( _compress_algo == 0 ){
      print(msg::kERROR,__FUNCTION__,"Compression Algorithm Not Set! Exiting");
      return false;
    }

    // Otherwise Get RawDigits and execute compression
    auto event_wf = storage->get_data<event_rawdigit>("daq");
    // If raw_digits object is empty -> exit
    if(!event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }

    // clear place-holder for new, compressed, waveforms
    _out_event_wf.clear();

    // reset variables that hold compression factor
    _inTicks  = 0;
    _outTicks = 0;
    
    // Loop over all waveforms
    _loopwatch.Start();
    for (size_t i=0; i<event_wf->size(); i++){
      
      //get tpc_data
      larlite::rawdigit* tpc_data = (&(event_wf->at(i)));      
      //Check for empty waveforms!
      if(tpc_data->ADCs().size()<1){
	print(msg::kERROR,__FUNCTION__,
	      Form("Found 0-length waveform: Event %d ... Ch. %d",event_wf->event_id(),tpc_data->Channel()));
	continue;
      }//if wf size < 1

      // Figure out channel's plane:
      // used because different planes will have different "buffers"
      UInt_t ch = tpc_data->Channel();
      int pl = larutil::Geometry::GetME()->ChannelToPlane(ch);

      // finally, apply compression:
      // *-------------------------*
      // 1) Convert tpc_data object to just the vector of shorts which make up the ADC ticks
      _watch.Start();
      const std::vector<short> ADCwaveform = tpc_data->ADCs();
      _time_get += _watch.RealTime();
      // 2) Now apply the compression algorithm. _compress_algo is an instance of CompressionAlgoBase
      _watch.Start();
      _compress_algo->ApplyCompression(ADCwaveform,pl,ch);
      _time_algo += _watch.RealTime();
      // 3) Retrieve output ranges saved
      auto const ranges = _compress_algo->GetOutputRanges();
      // 6) Study the Compression results for this channel
      _watch.Start();
      _compress_study->StudyCompression(ADCwaveform, ranges, pl);
      _time_study += _watch.RealTime();
      // 7) Calculate compression factor [ for now Ticks After / Ticks Before ]
      _watch.Start();
      CalculateCompression(ADCwaveform, ranges, pl);
      _time_calc += _watch.RealTime();
      // 8) clear _InWF and _OutWF from compression algo object -> resetting algorithm for next time it is called
      _compress_algo->Reset();
      // 9) Replace .root data file *event_wf* with new waveforms
      _watch.Start();
      if (_saveOutput)
	SwapData(tpc_data, ranges);
      _time_swap += _watch.RealTime();
	
    }//for all waveforms
    _time_loop += _loopwatch.RealTime();

    _compressionU /= _NplU;
    _compressionV /= _NplV;
    _compressionY /= _NplY;
    _compression  /= (_NplU+_NplV+_NplY);
    _compress_tree->Fill();
    _NplU = _NplV = _NplY = 0;
    _compressionU = _compressionV = _compressionY = 0;
    _evt += 1;
    
    //now take new WFs and place in event_wf vector
    if (_saveOutput){
      event_wf->clear();
      for (size_t m=0; m < _out_event_wf.size(); m++)
	event_wf->push_back(_out_event_wf.at(m));
    }
    return true;
  }

  bool ExecuteCompression::finalize() {

    double tottime = _evtwatch.RealTime();
    
    std::cout.precision(3);
    //std::scientific;

    std::cout << "  \033[95m<<Tot Time>>\033[00m  : " << tottime << " [s] ... or "
	      << tottime/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Loop Time>>\033[00m : " << _time_loop << " [s] ... or "
	      << _time_loop/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Get Time>>\033[00m  : " << _time_get << " [s] ... or "
	      << _time_get/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Algo Time>>\033[00m : " << _time_algo << " [s] ... or "
	      << _time_algo/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Study Time>>\033[00m: " << _time_study << " [s] ... or "
	      << _time_study/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Calc Time>>\033[00m : " << _time_calc << " [s] ... or "
	      << _time_calc/_evt << " [s/evt]" << std::endl
	      << "  \033[95m<<Swap Time>>\033[00m : " << _time_swap << " [s] ... or "
	      << _time_swap/_evt << " [s/evt]" << std::endl;

    _compress_algo->EndProcess(_fout);
    _compress_study->EndProcess(_fout);
    _compress_tree->Write();

    return true;
  }

  void ExecuteCompression::SwapData(const larlite::rawdigit *tpc_data,
				    const std::vector<std::pair< compress::tick, compress::tick> > &ranges)
  {

    // In this function we are taking the old larlite::rawdigit object and replacing it with
    // new larlite::rawdigit objects, one per output waveform coming from compression
    // Most of the information for this object (channel number, wire plane, etc) stays the same
    // What changes is the actual list of ADC counts, and the start time of the vector.

    UInt_t chan = tpc_data->Channel();


    //loop over new waveforms created
    for (size_t n=0; n < ranges.size(); n++){
      // prepare output waveform
      compress::tick t;
      std::vector<short> out;
      for (t = ranges[n].first; t < ranges[n].second; t++)
	out.push_back(*t);
      
      larlite::rawdigit new_tpc_data( chan, out.size(), out, larlite::raw::kNone);
      new_tpc_data.SetPedestal(ranges[n].first-_compress_algo->GetInputBegin());
      _out_event_wf.push_back(new_tpc_data);
    }

    return;
  }

  
  void ExecuteCompression::CalculateCompression(const std::vector<short> &beforeADCs,
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

  


}
#endif

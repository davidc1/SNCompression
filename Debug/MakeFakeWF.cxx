#ifndef LARLITE_MAKEFAKEWF_CXX
#define LARLITE_MAKEFAKEWF_CXX

#include "MakeFakeWF.h"
#include "DataFormat/rawdigit.h"


namespace larlite {

  bool MakeFakeWF::initialize() {

    return true;
  }
  
  bool MakeFakeWF::analyze(storage_manager* storage) {
  

    auto ev_rawwf = storage->get_data<event_rawdigit>("daq");

    // loop throguh user-provided waveforms and create a larlite::RawDigit object for each.
    // then save the waveforms to output file
    for (std::map<size_t, std::vector<short> >::iterator it=_wf_map.begin();
	 it != _wf_map.end(); ++it) {

      auto chan = it->first;
      auto adcv = it->second;

      larlite::rawdigit thischan(chan, adcv.size(), adcv, larlite::raw::Compress_t::kNone);

      ev_rawwf->emplace_back(thischan);
      
    }
    
    return true;
  }

  bool MakeFakeWF::finalize() {

    return true;
  }

  void MakeFakeWF::addWaveform(const size_t& channel, const std::vector<short>& adc_v) {
    
    _wf_map[channel] = adc_v;
    
    return;
    
  }

}
#endif

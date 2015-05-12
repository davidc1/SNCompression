#ifndef HITMAKER_CXX
#define HITMAKER_CXX

#include "HitMaker.h"

//Author:  David Caratelli
//Contact: dcaratelli@nevis.columbia.edu
//Date:    Feb 2014
//*********************************************************************
//This module takes waveforms and very simply transforms them into hits.
//Charge is counted up in terms of ADCs, not electrons.
//Various planes determined poorly since tpcfifo data does not store
//that information.
//Not all hit info is recorded .Only important part for me now is:
//time, charge, wire number.
//Doing this to be able to use HitViewer 
//*********************************************************************

namespace larlite {

  bool HitMaker::initialize() {

    _threshold = 6; //ADCs
    _evtNum = 0;
    return true;
  }
  
  bool HitMaker::analyze(storage_manager* storage) {

    //make vector to hold new hits
    auto hits = storage->get_data<event_hit>("daqhit");
    hits->clear();
    storage->set_id(hits->run(),hits->subrun(),hits->event_id());
    
    //read waveforms from event
    auto event_wf = storage->get_data<event_rawdigit>("daq");
    //make sure not empty...if so report
    if(!event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }
    
    //Loop over all waveforms
    for (size_t i=0; i<event_wf->size(); i++){
      
      //get tpc_data
      larlite::rawdigit* tpc_data = (&(event_wf->at(i)));      

      //Check for empty waveforms!
      if(tpc_data->ADCs().size()<1){
	print(msg::kERROR,__FUNCTION__,
	      Form("Found 0-length waveform: Event %d ... Ch. %d",event_wf->event_id(),tpc_data->Channel()));
	continue;
      }
      UInt_t chan = tpc_data->Channel();		
      //determine baseline based on plane-type
      if ( larutil::Geometry::GetME()->SignalType(chan) == larlite::geo::kCollection )
	_baseline = 400;
      else if ( larutil::Geometry::GetME()->SignalType(chan) == larlite::geo::kInduction )
	_baseline = 2048;
      
      //loop over samples
      bool active = false;
      double startT = 0.;
      double endT   = 0.;
      double peakT  = 0.;
      int Qpeak  = 0;
      int Qarea  = 0;
      auto const ADCs = tpc_data->ADCs();
      for (UShort_t adc_index=0; adc_index < ADCs.size(); adc_index++){
	int adcs = ADCs[adc_index];
	if ( ((adcs-_baseline) >= _threshold) ) {
	  if (!active) { startT = tpc_data->GetPedestal()+1+adc_index; }
	  active = true;
	  Qarea   += (adcs-_baseline);
	  //find if pulse peak
	  if ( (adcs-_baseline) > Qpeak ){
	    peakT = tpc_data->GetPedestal()+1+adc_index;
	    Qpeak = (adcs-_baseline);
	  }
	}
	else {//below threshold
	  if (active){//if we were in the middle of a hit, set end time & reset
	    endT = tpc_data->GetPedestal()+1+adc_index;
	    //fill hit information
	    if ( Qarea > 0 ){
	      _Hit.set_view(larutil::Geometry::GetME()->PlaneToView(larutil::Geometry::GetME()->ChannelToPlane(chan)));
	      _Hit.set_integral(Qarea, 0);
	      _Hit.set_amplitude(Qpeak, 0);
	      _Hit.set_time_range(startT, endT);
	      _Hit.set_time_peak(peakT, 0);
	      _Hit.set_channel(tpc_data->Channel());
	      _Hit.set_wire(larutil::Geometry::GetME()->ChannelToWireID(chan));
	      _Hit.set_multiplicity(0);
	      _Hit.set_goodness(1);
	      //add hit to hit vector
	      hits->push_back(_Hit);
	    }//fill hit info if area > 0	    
	    //reset values
	    Qarea = 0;
	    Qpeak = 0;
	    startT = 0;
	    endT   = 0;
	    peakT  = 0;
	    active = false;
	  }//if active was ON and we were in hit
	}//if below threshold
      }//loop over bins in this WF
      
    }//loop over waveforms in event
    
    //eliminate waveform
    event_wf->clear();

    _evtNum += 1;
    
    return true;
  }
  
  bool HitMaker::finalize() {

    return true;
  }
}
#endif

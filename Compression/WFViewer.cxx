#ifndef WFVIEWER_CXX
#define WFVIEWER_CXX

#include "WFViewer.h"

namespace larlite {


  //********************************
  WFViewer::WFViewer(): ana_base(), _hHits_U(nullptr), _hHits_V(nullptr), _hHits_Y(nullptr)
  //********************************
  {
    //Class Name
    _name = "WFViewer";

    _w2cm = larutil::GeometryHelper::GetME()->WireToCm();
    _t2cm = larutil::GeometryHelper::GetME()->TimeToCm();
    
    _useCmCm = true;
    
    _evtNum = 0;
    
  }

  //********************************
  bool WFViewer::initialize()
  //********************************
  {

    if (!_useCmCm){
      _w2cm = 1;
      _t2cm = 1;
    }

    return true;
  }
  

  //**********************************************
  bool WFViewer::analyze(storage_manager* storage)
  //**********************************************
  {

    //clean up histograms if they already exist (from previous event)
    if (_hHits_U) {delete _hHits_U; };  
    if (_hHits_V) {delete _hHits_V; };  
    if (_hHits_Y) {delete _hHits_Y; };  


    
    //read waveforms from event
    auto event_wf = storage->get_data<event_rawdigit>("daq");
    //make sure not empty...if so report
    if(!event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }
    std::cout << event_wf->at(0).ADCs().size() << std::endl;
    //if all ok, plot wire vs. time for hits
    // set RawDigit length to the size of the 1st RawDigit object
    _hHits_U = Prepare2DHisto(Form("Event %i - WF ADCs U-Plane",_evtNum),
			      0, larutil::Geometry::GetME()->Nwires(0)*_w2cm, 0,
			      event_wf->at(0).ADCs().size()*_t2cm, 0);
    _hHits_V = Prepare2DHisto(Form("Event %i - WF ADCs V-Plane",_evtNum),
			      0, larutil::Geometry::GetME()->Nwires(1)*_w2cm, 0,
			      event_wf->at(0).ADCs().size()*_t2cm, 1);
    _hHits_Y = Prepare2DHisto(Form("Event %i - WF ADCs Y-Plane",_evtNum),
			      0, larutil::Geometry::GetME()->Nwires(2)*_w2cm, 0,
			      event_wf->at(0).ADCs().size()*_t2cm, 2);

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
      

      //get WF plane to determine baseline
      UInt_t chan = tpc_data->Channel();
      if (chan >= 8254)
	continue;
      int time = 0;
      std::vector<short> ADCs = tpc_data->ADCs();
      //determine baseline based on plane-type
      if ( larutil::Geometry::GetME()->SignalType(chan) == larlite::geo::kCollection )
	_baseline = 400;
      else if ( larutil::Geometry::GetME()->SignalType(chan) == larlite::geo::kInduction )
	_baseline = 2048;

      if ( larlite::geo::kU == larutil::Geometry::GetME()->ChannelToPlane(chan) ){
	for (size_t u=0; u < ADCs.size(); u++){
	  _hHits_U->Fill( larutil::Geometry::GetME()->ChannelToWire(chan)*_w2cm, (time+u)*_t2cm, ADCs[u] );
	}
      }
      if ( larlite::geo::kV == larutil::Geometry::GetME()->ChannelToPlane(chan) ){
	for (size_t v=0; v < ADCs.size(); v++){
	  _hHits_V->Fill( larutil::Geometry::GetME()->ChannelToWire(chan)*_w2cm, (time+v)*_t2cm, ADCs[v] );
	}
      }
      if ( larlite::geo::kZ == larutil::Geometry::GetME()->ChannelToPlane(chan) ){
	for (size_t y=0; y < ADCs.size(); y++){
	  _hHits_Y->Fill( larutil::Geometry::GetME()->ChannelToWire(chan)*_w2cm, (time+y)*_t2cm, ADCs[y] );
	}
      }

    }//loop over all waveforms

    _evtNum += 1;
    
    return true;
  }

  //****************************************************************
  TH2S* WFViewer::Prepare2DHisto(std::string name, 
				 double wiremin, double wiremax,
				 double timemin, double timemax, int pl)
  //****************************************************************
  {
    
    TH2S* h = new TH2S(name.c_str(),name.c_str(),
		 larutil::Geometry::GetME()->Nwires(pl),  wiremin, wiremax,
		 timemax,  timemin, timemax);

    h->SetXTitle("Wire [cm]        ");
    h->SetYTitle("Time [cm]");
        
    return h;
  }

  bool WFViewer::finalize() {
  
    return true;
  }
}
#endif

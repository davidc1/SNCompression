#ifndef HITVIEWER_CXX
#define HITVIEWER_CXX

#include "HitViewer.h"

namespace larlite {


  //********************************
  HitViewer::HitViewer(): ana_base(), _hHits_U(), _hHits_V(), _hHits_Y()
  //********************************
  {
    //Class Name
    _name = "HitViewer";
    //set initialization for pointers
    _hHits_U = 0;
    _hHits_V = 0;
    _hHits_Y = 0;

    _w2cm = larutil::GeometryUtilities::GetME()->WireToCm();
    _t2cm = larutil::GeometryUtilities::GetME()->TimeToCm();

    _evtNum = 0;
    
  }

  //********************************
  bool HitViewer::initialize()
  //********************************
  {

    return true;
  }
  

  //**********************************************
  bool HitViewer::analyze(storage_manager* storage)
  //**********************************************
  {

    //clean up histograms if they already exist (from previous event)
    if (_hHits_U) {delete _hHits_U; _hHits_U = 0;};  
    if (_hHits_V) {delete _hHits_V; _hHits_V = 0;};  
    if (_hHits_Y) {delete _hHits_Y; _hHits_Y = 0;};  

    //Get Hits
    auto hits = storage->get_data<event_hit>("daqhit");
    //Define axis ranges
    std::vector<double> chmax, chmin, wiremax, wiremin, timemax, timemin;
    //Find axis boundary
    GetAxisRange(chmax, chmin, wiremax, wiremin, timemax, timemin, hits);
    //proceed only if values actually reset
    if ( wiremax[0] <= -1 )
      {
	print(msg::kWARNING,__FUNCTION__,
	      "Did not find any reconstructed hits in view 0. Skipping this event...");
	return true;
      }
    
    //if all ok, plot wire vs. time for hits
    _hHits_U = Prepare2DHisto(Form("Event %i - Hit Charge [ Area in ADCs ] U-Plane",_evtNum),
			      wiremin[0]*_w2cm, wiremax[0]*_w2cm, timemin[0]*_t2cm, timemax[0]*_t2cm);
    _hHits_V = Prepare2DHisto(Form("Event %i - Hit Charge [ Area in ADCs ] V-Plane",_evtNum),
			      wiremin[1]*_w2cm, wiremax[1]*_w2cm, timemin[1]*_t2cm, timemax[1]*_t2cm);
    _hHits_Y = Prepare2DHisto(Form("Event %i - Hit Charge [ Area in ADCs ] Y-Plane",_evtNum),
			      wiremin[2]*_w2cm, wiremax[2]*_w2cm, timemin[2]*_t2cm, timemax[2]*_t2cm);
    
    //loop over hits
    for (size_t i=0; i<hits->size(); i++)
      {
	const hit *this_hit = (&(hits->at(i)));
	//place in right plane
	if ( this_hit->View()==0 )
	  _hHits_U->Fill( this_hit->WireID().Wire*_w2cm, this_hit->PeakTime()*_t2cm, this_hit->Integral() );
	if ( this_hit->View()==1 )
	  _hHits_V->Fill( this_hit->WireID().Wire*_w2cm, this_hit->PeakTime()*_t2cm, this_hit->Integral() );
	if ( this_hit->View()==2 )
	  _hHits_Y->Fill( this_hit->WireID().Wire*_w2cm, this_hit->PeakTime()*_t2cm, this_hit->Integral() );
	
      }//end loop over hits
    
    _evtNum +=1;
    
    return true;
  }

  //****************************************************************
  TH2I* HitViewer::Prepare2DHisto(std::string name, 
				      double wiremin, double wiremax,
				      double timemin, double timemax)
  //****************************************************************
  {
    
    TH2I* h=0;
    if(h) delete h;
    
    h = new TH2I("2DViewer", name.c_str(),
		 100,  wiremin-10, wiremax+10,
		 100,  timemin-10, timemax+10);

    h->SetXTitle("Wire [cm]        ");
    h->SetYTitle("Time [cm]");
    
    return h;
  }

  bool HitViewer::finalize() {
  
    return true;
  }


  //**********************************************
  void HitViewer::GetAxisRange(std::vector<Double_t> &chmax,
			       std::vector<Double_t> &chmin,
			       std::vector<Double_t> &wiremax,
			       std::vector<Double_t> &wiremin,
			       std::vector<Double_t> &timemax,
			       std::vector<Double_t> &timemin,
			       const event_hit* hits) const
  {

    // Make sure input vector is of size wire plane with initial value -1 (if not yet set)                                                       
    chmax.resize((larlite::geo::kW+1),-1);
    wiremax.resize((larlite::geo::kW+1),-1);
    timemax.resize((larlite::geo::kW+1),-1);
    chmin.resize((larlite::geo::kW+1),-1);
    wiremin.resize((larlite::geo::kW+1),-1);
    timemin.resize((larlite::geo::kW+1),-1);

    for (size_t i=0; i < hits->size(); i++){

      const hit *h = (&(hits->at(i)));

      larlite::geo::View_t  view    =  h->View();
      Double_t     wire    =  (Double_t)(h->WireID().Wire);
      Double_t     ch      =  (Double_t)(h->Channel());
      Double_t     tstart  =  h->StartTick();
      Double_t     tend    =  h->EndTick();

      if( wiremax[view] < 0 || wiremax[view] < wire )  wiremax[view] = wire;
      if( chmax[view]   < 0 || chmax[view]   < ch   )  chmax[view]   = ch;
      if( timemax[view] < 0 || timemax[view] < tend )  timemax[view] = tend;
      
      if( wiremin[view] < 0 || wiremin[view] > wire   )  wiremin[view] = wire;
      if( chmin[view]   < 0 || chmin[view]   > ch     )  chmin[view]   = ch;
      if( timemin[view] < 0 || timemin[view] > tstart )  timemin[view] = tstart;
    }
    
    return;
  }

}
#endif

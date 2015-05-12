#ifndef COMPRESSIONSTUDYHITS_CXX
#define COMPRESSIONSTUDYHITS_CXX

#include "CompressionStudyHits.h"

namespace compress {


  CompressionStudyHits::CompressionStudyHits()
    : CompressionStudyBase()
    , _in_study_tree(nullptr)
    , _out_study_tree(nullptr)
  {
    
    // Threshold to decleare a "hit"
    _threshold = 5;

    if (_in_study_tree) { delete _in_study_tree; }
    _in_study_tree = new TTree("in_study_tree","Input Study Tree");
    _in_study_tree->Branch("_hit_peak",&_hit_peak,"hit_peak/D");
    _in_study_tree->Branch("_hit_area",&_hit_area,"hit_area/D");
    _in_study_tree->Branch("_pl",&_pl,"pl/I");
    if (_out_study_tree) { delete _out_study_tree; }
    _out_study_tree = new TTree("out_study_tree","Output Study Tree");
    _out_study_tree->Branch("_hit_peak",&_hit_peak,"hit_peak/D");
    _out_study_tree->Branch("_hit_area",&_hit_area,"hit_area/D");
    _out_study_tree->Branch("_pl",&_pl,"pl/I");

    return;
  }

  void CompressionStudyHits::StudyCompression(const std::vector<short> &in,
					      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
					      const int pl)
  {

    _pl = pl;

    findHits(std::make_pair(in.begin(),in.end()),true);
    for (auto &range : ranges)
      findHits(range,false);
    
    return;
  }


  void CompressionStudyHits::findHits(const std::pair<tick,tick>& range, bool in){

    if ( (range.second-range.first) < 3)
      return;

    // iterator
    compress::tick t;
    // Baseline for the waveform
    double baseline = 0;
    for (t = range.first; t < range.first+3; t++)
      baseline += *t;
    baseline /= 3.;
    // Keep track of Area of Hit
    double Qarea = 0;
    // Keep track of Max Amplitude of Hit
    double Qpeak = 0;
    // Keep track of whether we are in an "active" region
    bool active = false;
    // Keep track of number of hits above threshold (minimum of 2 required)
    int Nabove = 0;

    for (t = range.first; t < range.second; t++){
      //for (size_t adc_index=0; adc_index < wf.size(); adc_index++){
	int adcs = *t;
	if ( PassThreshold(adcs,baseline) ) {
	  active = true;
	  Nabove += 1;
	  // if U plane -> filp
	  if ( _pl == 0){
	    Qarea   += -(adcs-baseline);
	    //find if pulse peak
	    if ( -(adcs-baseline) > Qpeak )
	      Qpeak = -(adcs-baseline);
	  }// if U plane
	  else{
	    Qarea   += (adcs-baseline);
	    //find if pulse peak
	    if ( (adcs-baseline) > Qpeak )
	      Qpeak = (adcs-baseline);
	  }// if V,Y planes
	}
	else {//below threshold
	  if ( active && (Nabove > 1) ){//if we were in the middle of a hit
	    //fill hit information
	    if ( Qarea > 0 ){
	      _hit_area = Qarea;
	      _hit_peak = Qpeak;
	      if (in)
		_in_study_tree->Fill();
	      else
		_out_study_tree->Fill();
	    }//fill hit info if area > 0	    
	    //reset values
	    Qarea = 0;
	    Qpeak = 0;
	    active = false;
	    Nabove = 0;
	  }//if active was ON and we were in hit
	}//if below threshold
    }//loop over bins in this WF
    
   
    return;
  }


  bool CompressionStudyHits::PassThreshold(double thisADC, double base){

    // if V & Y plane
    if (_pl != 0){
      if (thisADC > (base + _threshold) )
	return true;
    }
    // if U plane
    else{
      if (thisADC < (base - _threshold) )
	return true;
    }
    
    return false;
  }
  
  
  void CompressionStudyHits::EndProcess(TFile *fout){

    if (fout){
      if (_in_study_tree)
	_in_study_tree->Write();
      if (_out_study_tree)
	_out_study_tree->Write();
    }
    return;
  }
  
}

#endif

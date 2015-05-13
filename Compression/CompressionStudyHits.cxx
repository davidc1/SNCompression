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
    _in_study_tree->Branch("_hit_peakOut",&_hit_peakOut,"hit_peakOut/D");
    _in_study_tree->Branch("_hit_areaOut",&_hit_areaOut,"hit_areaOut/D");
    _in_study_tree->Branch("_isSaved",&_isSaved,"isSaved/I");
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

    // function to match a hit across input and output regions:
    // find hits in input and see if they are present in the output as well
    matchHits(std::make_pair(in.begin(),in.end()),ranges);
    /*
    findHits(std::make_pair(in.begin(),in.end()),true);
    for (auto &range : ranges)
      findHits(range,false);
    */
    return;
  }


  void CompressionStudyHits::matchHits(const std::pair<tick,tick>& inrange,
				       const std::vector<std::pair<tick,tick> >& outranges){

    // keep track of the region in the compressed output that we have last reached
    size_t currentPair = 0;
    // this value can be updated once we realize we have reached a new pair

    if ( (inrange.second-inrange.first) < 3)
      return;

    // iterator
    compress::tick t;
    // Baseline for the waveform
    double baseline = 0;
    for (t = inrange.first; t < inrange.first+3; t++)
      baseline += *t;
    baseline /= 3.;
    // Keep track of Area of Hit
    double Qarea    = 0;
    double QareaOut = 0;
    // Keep track of Max Amplitude of Hit
    double Qpeak    = 0;
    double QpeakOut = 0;
    // Keep track of whether we are in an "active" region
    bool active = false;
    // Keep track of number of hits above threshold (minimum of 2 required)
    int Nabove = 0;

    for (t = inrange.first; t < inrange.second; t++){
	int adcs = *t;
	if ( PassThreshold(adcs,baseline) ) {
	  // figure out if this tick has been saved in the output
	  bool saved = isTickInOutput(t,outranges,currentPair);
	  //if (saved) { std::cout << "Yes! saved...Tick value: " << *t << std::endl; }
	  saved ? _isSaved = 1 : _isSaved = 0;
	  active = true;
	  Nabove += 1;
	  // if U plane -> filp
	  if ( _pl == 0){
	    Qarea   += -(adcs-baseline);
	    if (saved) { QareaOut += -(adcs-baseline); }
	    //find if pulse peak
	    if ( -(adcs-baseline) > Qpeak ){
	      Qpeak = -(adcs-baseline);
	      if (saved) { QpeakOut = -(adcs-baseline); }
	    }
	  }// if U plane
	  else{
	    Qarea   += (adcs-baseline);
	    if (saved) { QareaOut += (adcs-baseline); }
	    //find if pulse peak
	    if ( (adcs-baseline) > Qpeak ){
	      Qpeak = (adcs-baseline);
	      if (saved) { QpeakOut = (adcs-baseline); }
	    }
	  }// if V,Y planes
	}
	else {//below threshold
	  if ( active && (Nabove > 1) ){//if we were in the middle of a hit
	    //fill hit information
	    if ( Qarea > 0 ){
	      _hit_area = Qarea;
	      _hit_peak = Qpeak;
	      _hit_areaOut = QareaOut;
	      _hit_peakOut = QpeakOut;
	      _in_study_tree->Fill();
	    }//fill hit info if area > 0	    
	    //reset values
	    Qarea = 0;
	    Qpeak = 0;
	    QareaOut = 0;
	    QpeakOut = 0;
	    active = false;
	    Nabove = 0;
	  }//if active was ON and we were in hit
	}//if below threshold
    }//loop over bins in this WF

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

  bool CompressionStudyHits::isTickInOutput(const compress::tick& t,
					    const std::vector<std::pair<tick,tick> >& outranges, size_t& currentPair)
  {

    // if no outranges -> always false!
    if (outranges.size() == 0)
      return false;

    //std::cout << "Tick Value : " << *t << std::endl;
    //std::cout << "Current pair values : " << *(outranges[currentPair].first) << ", "  << *(outranges[currentPair].second) << std::endl;
    
    //std::cout << "Distance current start & this tick: " << std::distance(t,outranges[currentPair].first) << std::endl;

    // are we in the current pair?
    if ( (std::distance(outranges[currentPair].first,t) > 0) and 
	 (std::distance(outranges[currentPair].second,t) < 0) )
      return true;
    
    // are we beyond the current pair?
    else if (t > outranges[currentPair].second){
      // are we in the next pair?
      if (currentPair < (outranges.size()-1)){
	if ( (std::distance(outranges[currentPair+1].first,t) > 0) and 
	     (std::distance(outranges[currentPair+1].second,t) < 0) ){
	  // yes we are in the next pair! update currentPair
	  currentPair +=1;
	  return true;
	}// if in the next pair
      }// if there is a next pair
    }// if we are beyond the first pair
    
    return false;
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

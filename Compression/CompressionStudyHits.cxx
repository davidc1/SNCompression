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
    _threshold = 3;
    // consecutive ticks for a hit
    _consecutive = 3;

    if (_in_study_tree) { delete _in_study_tree; }
    _in_study_tree = new TTree("in_study_tree","Input Study Tree");
    _in_study_tree->Branch("_hit_peak",&_hit_peak,"hit_peak/D");
    _in_study_tree->Branch("_hit_area",&_hit_area,"hit_area/D");
    _in_study_tree->Branch("_hit_peakOut",&_hit_peakOut,"hit_peakOut/D");
    _in_study_tree->Branch("_hit_areaOut",&_hit_areaOut,"hit_areaOut/D");
    _in_study_tree->Branch("_isSaved",&_isSaved,"isSaved/I");
    _in_study_tree->Branch("_baseline",&_baseline,"baseline/D");
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

    // find the Baseline for the waveform
    // average 20 ticks and keep baseline once we find
    // a stable region (below some variance cut)
    _baseline = 0;
    double baseline = 0;
    double variance = 0;
    int offset = 0;
    while (_baseline == 0){
      for (t = inrange.first+offset; t < inrange.first+20+offset; t++)
	baseline += *t;
      baseline /= 20.;
      for (t = inrange.first+offset; t < inrange.first+20+offset; t++)
	variance += (*t-baseline)*(*t-baseline);
      variance = sqrt(variance/20.);
      //std::cout << "offset: " << offset << "\tBase: " << baseline << "\tVar: " << variance << std::endl;
      if (variance < 1)
	_baseline = baseline;
      variance = 0;
      baseline = 0;
      offset += 20;
    }
    // keep track of start and end ticks
    int startT = 0;
    int endT   = 0;
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
	if ( PassThreshold(adcs,_baseline) ) {
	  // figure out if this tick has been saved in the output
	  bool saved = isTickInOutput(t,outranges,currentPair);
	  saved ? _isSaved = 1 : _isSaved = 0;
	  if (!active) { startT = std::distance(inrange.first,t); }
	  // we are in an active region
	  active = true;
	  Nabove += 1;
	  // if U plane -> filp
	  if ( _pl == 0){
	    Qarea   += -(adcs-_baseline);
	    if (saved) { QareaOut += -(adcs-_baseline); }
	    //find if pulse peak
	    if ( -(adcs-_baseline) > Qpeak ){
	      Qpeak = -(adcs-_baseline);
	      if (saved) { QpeakOut = -(adcs-_baseline); }
	    }
	  }// if U plane
	  else{
	    Qarea   += (adcs-_baseline);
	    if (saved) { QareaOut += (adcs-_baseline); }
	    //find if pulse peak
	    if ( (adcs-_baseline) > Qpeak ){
	      Qpeak = (adcs-_baseline);
	      if (saved) { QpeakOut = (adcs-_baseline); }
	    }
	  }// if V,Y planes
	}
	else {//below threshold
	  if ( active && (Nabove > _consecutive) ){//if we were in the middle of a hit
	    endT = std::distance(inrange.first,t);
	    //fill hit information
	    if ( Qarea > 0 ){
	      if (_verbose){
		std::cout << "Found Hit: [" << startT << ", " << endT << "]" << std::endl
			  << "[Area, Peak]. Input: [" << Qarea << ", " << Qpeak << "]\tOutput: ["
			  << QareaOut << ", " << QpeakOut << "]" << std::endl
			  << "CurrentPair: " << currentPair << "\tTot pairs: " << outranges.size() << std::endl;
	      }
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

    // if the current pair is beyond the size of output ranges -> return false
    if (currentPair >= outranges.size())
      return false;

    // are we in the current pair?
    if ( (std::distance(outranges[currentPair].first,t) > 0) and 
	 (std::distance(outranges[currentPair].second,t) < 0) )
      return true;
    
    // are we beyond the current pair?
    while(std::distance(outranges[currentPair].second,t) > 0){
      currentPair += 1;
      // are we in the next pair? -> keep searching untill we exhaust pairs
      if (currentPair < outranges.size() ){
	if ( (std::distance(outranges[currentPair].first,t) > 0) and 
	     (std::distance(outranges[currentPair].second,t) < 0) ){
	  // yes we are in the next pair! update currentPair
	  return true;
	}// if in the next pair
      }// if there is a next pair
      else { return false; }
    }// if we are beyond the first pair searched
    
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

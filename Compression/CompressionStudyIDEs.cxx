#ifndef COMPRESSIONSTUDYIDES_CXX
#define COMPRESSIONSTUDYIDES_CXX

#include "CompressionStudyIDEs.h"

namespace compress {

  CompressionStudyIDEs::CompressionStudyIDEs()
    : _ide_study(nullptr)
  {
    if (_ide_study) { delete _ide_study; }
    _ide_study = new TTree("ide_study","IDE Study Tree");
    _ide_study->Branch("_ideE",&_ideE,"ideE/D");
    _ide_study->Branch("_idePeak",&_idePeak,"idePeak/D");
    _ide_study->Branch("_ideEout",&_ideEout,"ideEout/D");
    _ide_study->Branch("_pl",&_pl,"pl/I");
    _ide_study->Branch("_start",&_start,"start/I");
    _ide_study->Branch("_end",&_end,"end/I");
    _ide_study->Branch("_ch",&_ch,"ch/I");
    _ide_study->Branch("_evt",&_evt,"evt/I");
    _verbose = false; 
  }
  
  void CompressionStudyIDEs::StudyCompression(const std::vector<std::pair<unsigned short, double> >& IDEs,
					      const std::pair<compress::tick,compress::tick>& range,
					      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
					      const int pl, const int ch, const int evt)
  {

    _pl = pl;
    _ch = ch;
    _evt = evt;

    if (_verbose) { std::cout << "Evt: " << evt << " Ch: " << ch << std::endl; }
    
    // make a vector the length of the total waveform
    // fill it with the IDEs at the appropriate time-ticks
    // search for IDE pulses and compare to the output waveform
    // to see if these IDEs were found by the compression algorithm
    std::vector<double> _ide_v(std::distance(range.first,range.second),0.);
    for (auto const& ide : IDEs){
      if (ide.first < _ide_v.size())
	_ide_v[ide.first] += ide.second;
    }

    compress::tick t;
    _ideE = 0;
    _idePeak = 0;
    _ideEout = 0;
    size_t currentPair = 0;
    bool saved = false;
    bool active = false;
    _start = 0;
    _end = 0;
    
    // loop thorugh the vector searching for IDE pulses
    for (t = range.first; t < range.second; t++){
      size_t pos = std::distance(range.first,t);
      if (_ide_v[pos] > 0.){
	if (!active) { _start = pos; }
	active = true;
	// active region
	_ideE += _ide_v[pos];
	if (_ide_v[pos] > _idePeak)
	  _idePeak = _ide_v[pos];
	// is this tick in the output?
	saved = isTickInOutput(t,ranges,currentPair);
	if (saved)
	  _ideEout += _ide_v[pos];
      }// if in IDE pulse
      else{
	if (active){
	  // if we were in an active region
	  _end = pos;
	  _ide_study->Fill();
	  if (_verbose){
	    std::cout << "IDE region: [" << _start << ", " << _end << "]"
		      << "\tIDE E: " << _ideE << "\tSaved: " << _ideEout << std::endl;
	  }
	  active = false;
	  _ideE = 0;
	  _ideEout = 0;
	  _idePeak = 0;
	  _start = 0;
	  _end = 0;
	}// if active
      }// if in non-active region
    }//scan vector

    return;
  }


  bool CompressionStudyIDEs::isTickInOutput(const compress::tick& t,
					    const std::vector<std::pair<compress::tick,compress::tick> >& outranges,
					    size_t& currentPair)
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

  void CompressionStudyIDEs::EndProcess(TFile *fout){

    if (fout){
      if (_ide_study)
      	_ide_study->Write();
    }
    return;
  }
  
}

#endif

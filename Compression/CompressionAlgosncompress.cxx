#ifndef COMPRESSIONALGOSNCOMPRESS_CXX
#define COMPRESSIONALGOSNCOMPRESS_CXX

#include "CompressionAlgosncompress.h"

namespace compress {
  

  CompressionAlgosncompress::CompressionAlgosncompress()
    : CompressionAlgoBase()
    , _algo_tree(nullptr)
  {
    _maxADC = 4095;
    _buffer.reserve(3);
    _buffer[0].reserve(2);
    _buffer[1].reserve(2);
    _buffer[2].reserve(2);
    _thresh.reserve(3);
    std::vector<std::vector<int> > tmp(3,std::vector<int>(2,0));
    _buffer = tmp;

    _fillTree = false;

    //Setup tree
    if (_algo_tree) { delete _algo_tree; }
    _algo_tree = new TTree("_algo_tree","Algorithm-specific Tree");
    _algo_tree->Branch("_pl",&_pl,"pl/I");
    _algo_tree->Branch("_v1",&_v1,"v1/D");
    _algo_tree->Branch("_v2",&_v2,"v2/D");
    _algo_tree->Branch("_v3",&_v3,"v3/D");
    _algo_tree->Branch("_b1",&_b1,"b1/D");
    _algo_tree->Branch("_b2",&_b2,"b2/D");
    _algo_tree->Branch("_b3",&_b3,"b3/D");
    _algo_tree->Branch("_max",&_max,"max/D");
    _algo_tree->Branch("_interesting",&_interesting,"interesting/I");
    _algo_tree->Branch("_save",&_save,"save/I");
    
  }
  
  void CompressionAlgosncompress::SetUVYplaneBuffer(int upre, int upost, int vpre, int vpost, int ypre, int ypost){
    
    _buffer[0][0]  = upre;
    _buffer[0][1] = upost;
    _buffer[1][0]  = vpre;
    _buffer[1][1] = vpost;
    _buffer[2][0]  = ypre;
    _buffer[2][1] = ypost;
    return;
  }
  
  
  void CompressionAlgosncompress::ApplyCompression(const std::vector<short> &waveform, const int mode, const UInt_t ch){
    
    //double base = 0;
    int buffer = 0;
    
    // entries in the waveform:
    int nTicks = waveform.size();

    // iterator to the beginning and end of the waveform
    _begin = waveform.begin();
    _end   = waveform.end();
    
    std::vector<short> outputwf;
    _baselines.clear();
    _variances.clear();
    
    int diff = 0;
    // keep track of whether 3-block segment is "interesting" -> default is false
    bool interesting = false;
    
    double baseline[3];
    double variance[3];
    // Tick @ which waveform starts
    int start = 0;
    
    _pl = mode;

    // how many blocks? we need to make sure we don't overflow the window length
    int nsegments = int(waveform.size()/(float(_block)))-3;

    for(int pos = 0; pos < nsegments; pos+= 3){

      int thistick = pos*_block;
      
      _thisTick = _begin+pos*_block;

      if (_debug)
	std::cout << "position is: " << pos << std::endl;
      for(int j = 0; j<3; j++){
	variance[j] = 0;
	baseline[j] = 0;
      }
      //find baseline
      tick t = _thisTick;
      for (; t < _thisTick + _block; t++)
	baseline[0] += *t;
      for (; t < _thisTick + 2*_block; t++)
	baseline[1] += *t;
      for (; t < _thisTick + 3*_block; t++)
	baseline[2] += *t;

      for(int j = 0; j<3; j++)
	baseline[j] = baseline[j]/_block;
      
      //find variance
      t = _thisTick;
      for (; t < _thisTick + _block; t++){
	diff = *t-baseline[0];
	if (diff < _block-1) { variance[0] += diff*diff; }
	else { variance[0] += 4095; }
      }
      for (; t < _thisTick + 2*_block; t++){
	diff = *t-baseline[1];
	if (diff < _block-1) { variance[1] += diff*diff; }
	else { variance[1] += 4095; }
      }
      for (; t < _thisTick + 3*_block; t++){
	diff = *t-baseline[2];
	if (diff < _block-1) { variance[2] += diff*diff; }
	else { variance[2] += 4095; }
      }

      for(int j = 0; j<3; j++){
	variance[j] = variance[j]/_block;
	_baselines.push_back(baseline[j]);
	_variances.push_back(variance[j]);
      }
      
      _v1 = variance[0];
      _v2 = variance[1];
      _v3 = variance[2];
      _b1 = baseline[0];
      _b2 = baseline[1];
      _b3 = baseline[2];


      if (_debug){
	std::cout << "Baseline. Block 1: " << baseline[0] << "\tBlock 2: " << baseline[1] << "\tBlock 3: " << baseline[2] << std::endl;
	std::cout << "Variance. Block 1: " << variance[0] << "\tBlock 2: " << variance[1] << "\tBlock 3: " << variance[2] << std::endl;
      }
      

      // Now determine if these 3 blocks are interesting.
      // if so, try and look for a waveform within
      if ( ( (baseline[2] - baseline[1]) * (baseline[2] - baseline[1]) < _deltaB ) && 
	   ( (baseline[2] - baseline[0]) * (baseline[2] - baseline[0]) < _deltaB ) && 
	   ( (baseline[1] - baseline[0]) * (baseline[1] - baseline[0]) < _deltaB ) &&
	   ( (variance[2] - variance[1]) * (variance[2] - variance[1]) < _deltaV ) &&
	   ( (variance[2] - variance[0]) * (variance[2] - variance[0]) < _deltaV ) &&
	   ( (variance[1] - variance[0]) * (variance[1] - variance[0]) < _deltaV ) ){
	// no -> boring
	_baselineMap[ch] = baseline[1];
	//	base = baseline[1];
	interesting = false;
      }
      else{
	// yes! interesting
	interesting = true;
      }
      
      if (_verbose && interesting)
	std::cout << "Interesting @ tick " << thistick << std::endl;

      _interesting = 0;
      if (interesting) { _interesting = 1; }
      
      if (_verbose && interesting && (_baselineMap.find(ch) == _baselineMap.end()) )
	std::cout << "WARNING: interesting stuff but baseline has not yet been set for ch " << ch << std::endl;
      if (interesting && (_baselineMap.find(ch) != _baselineMap.end()) ){

	double base = _baselineMap[ch];
	
	// reset maxima
	_max = 0;

	// Then go through the 3 blocks again trying to find a waveform to save
	
	// save will keep track of tick at which waveform goes above threshold
	// == 0 if not -> use as logic method to decide if to push back or not
	_save = 0;
	// also keep track of each new sub-waveform to push back to output waveform
	outputwf.clear();
	// loop over all 3*_block ticks together applyting threshold _thresh
	// after basline subtraction w/ baseline "base"
	
	// start & end tick for each block saved
	tick s;
	tick e;
	std::pair<tick,tick> thisRange; 
	for (t = _thisTick; t < _thisTick + 3*_block; t++){

	  double thisADC = *t;
	  if (thisADC-base > _max) { _max = thisADC-base; }

	  if ( PassThreshold(thisADC, base) ){
	    if (_verbose) { std::cout << "+ "; }
	    _save = 1;
	    // yay -> active
	    // if start == 0 it means it's a new pulse! (previous tick was quiet)
	    // keep track of maxima
	    if ( start == 0 ){
	      start = int(t-_begin);
	      // also, since we just started...add "backwards ticks" to account for padding
	      if ( (t-_buffer[mode][0]) > _begin) { s = t-_buffer[mode][0]; }
	      else { s = _begin; }
	      if (_verbose) { std::cout << "found start-tick " << s-_begin << std::endl; }
	    }
	    // add bin content to temporary output waveform
	    outputwf.push_back(*t);
	  }
	  
	  else{
	    // we are in a sub-threshold region.
	    // 2 possibilities:
	    // 1) we were in a sub-threshold region at the previous tick -> then just carry on
	    // 2) we were in an active region in the previous tick -> we just "finished" this mini-waveform.
	    //    then Complete padding and save to output
	    if ( start > 0 ){
	      // finish padding
	      if ( (t+_buffer[mode][1]) < _end) { e = t+_buffer[mode][1]; }
	      else { e = _end; }
	      // push back waveform and time-tick
	      if (_verbose) {
		std::cout << std::endl;
		std::cout << "saving [" << s-_begin << ", " << e-_begin << "]" << std::endl;
	      }
	      // if the beginning is before the end of the previous section -> just expand
	      if (_timeRange.size() > 0){
		if (s < _timeRange.back().second){
		  // this new range starts before the last one ends -> edit the last one
		  _timeRange.back().second = e;
		}
		else{
		  thisRange = std::make_pair(s,e);
		  _timeRange.push_back(thisRange);
		}
	      }
	      else{
		thisRange = std::make_pair(s,e);
		_timeRange.push_back(thisRange);
	      }
	      _save = 0;
	      start = 0;
	    }
	  }
	  
	}

	// if we are still in the active region: save and reset
	if (start > 0){
	  // decide where the end is: at the end of this 3-block segment
	  e = _thisTick + 3*_block;
	  if (_verbose) {
	    std::cout << std::endl;
	    //std::cout << "saving [" << start-buffer << ", " << start-buffer+outputwf.size() << "]" << std::endl;
	    std::cout << "saving [" << s-_begin << ", " << e-_begin << "]" << std::endl;
	  }
	  // if the beginning is before the end of the previous section -> just expand
	  if (_timeRange.size() > 0){
	    if (s < _timeRange.back().second){
	      // this new range starts before the last one ends -> edit the last one
	      _timeRange.back().second = e;
	    }
	    else{
	      thisRange = std::make_pair(s,e);
	      _timeRange.push_back(thisRange);
	    }
	  }
	  else{
	    thisRange = std::make_pair(s,e);
	    _timeRange.push_back(thisRange);
	  }
	  _save = 0;
	  start = 0;
	}
	
	if (_verbose) { std::cout << std::endl; }
      }//if interesting!
      if (_fillTree)
	_algo_tree->Fill();
      
    }//for all 3-block segments

    return;
  }


  bool CompressionAlgosncompress::PassThreshold(double thisADC, double base){

    // if positive threshold
    if (_thresh[_pl] >= 0){
      if (thisADC > base + _thresh[_pl])
	return true;
    }
    // if negative threshold
    else{
      if (thisADC < base + _thresh[_pl])
	return true;
    }
    
    return false;
  }
  
  void CompressionAlgosncompress::EndProcess(TFile* fout){
    
    if (fout){
      if (_algo_tree)
	_algo_tree->Write();
    }

    return;
  }

}
#endif

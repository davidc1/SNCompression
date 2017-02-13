#ifndef COMPRESSIONALGOSNCOMPRESS_CXX
#define COMPRESSIONALGOSNCOMPRESS_CXX

#include "CompressionAlgosncompress.h"
#include <limits>
#include <cstddef>

namespace compress {
  

  CompressionAlgosncompress::CompressionAlgosncompress()
    : CompressionAlgoBase()
    , _algo_tree(nullptr)
  {
    _maxADC = 4095;
    _buffer = std::vector<std::vector<int>>(3,std::vector<int>(2,0));
    //_buffer.reserve(3);
    //_buffer[0].reserve(2);
    //_buffer[1].reserve(2);
    //_buffer[2].reserve(2);
    _thresh = std::vector<double>(3,0.);
    _pol = std::vector<int>(3,0.);
    //_thresh.reserve(3);
    std::vector<std::vector<int> > tmp(3,std::vector<int>(2,0));
    _buffer = tmp;

    _fillTree = false;

    //Setup tree
    if (_algo_tree) { delete _algo_tree; }
    _algo_tree = new TTree("_algo_tree","Algorithm-specific Tree");
    _algo_tree->Branch("_pl",&_pl,"pl/I");
    _algo_tree->Branch("_v1",&_v1,"v1/I");
    _algo_tree->Branch("_v2",&_v2,"v2/I");
    _algo_tree->Branch("_v3",&_v3,"v3/I");
    _algo_tree->Branch("_b1",&_b1,"b1/I");
    _algo_tree->Branch("_b2",&_b2,"b2/I");
    _algo_tree->Branch("_b3",&_b3,"b3/I");
    _algo_tree->Branch("_max",&_max,"max/D");
    _algo_tree->Branch("_save",&_save,"save/I");
 
//Anya edit: new variables: differences between b1/v1 and b2/v2 & b3/v3 during scan
    _algo_tree->Branch("_delb12",&_delb12,"delb12/I");
    _algo_tree->Branch("_delb13",&_delb13,"delb13/I");
    _algo_tree->Branch("_delv12",&_delv12,"delv12/I");
    _algo_tree->Branch("_delv13",&_delv13,"delv13/I");
    _algo_tree->Branch("_baseline_vals",&_baseline_vals,"baseline_vals/I");  
    _algo_tree->Branch("_variance_vals",&_variance_vals,"variance_vals/I");  
//U plane
    _algo_tree->Branch("_delb12u",&_delb12u,"delb12u/I");
    _algo_tree->Branch("_delb13u",&_delb13u,"delb13u/I");
    _algo_tree->Branch("_delv12u",&_delv12u,"delv12u/I");
    _algo_tree->Branch("_delv13u",&_delv13u,"delv13u/I");
    _algo_tree->Branch("_baseline_vals_u",&_baseline_vals_u,"baseline_vals_u/I");
    _algo_tree->Branch("_variance_vals_u",&_variance_vals_u,"variance_vals_u/I");   
//V plane
    _algo_tree->Branch("_delb12v",&_delb12v,"delb12v/I");
    _algo_tree->Branch("_delb13v",&_delb13v,"delb13v/I");
    _algo_tree->Branch("_delv12v",&_delv12v,"delv12v/I");
    _algo_tree->Branch("_delv13v",&_delv13v,"delv13v/I");
    _algo_tree->Branch("_baseline_vals_v",&_baseline_vals_v,"baseline_vals_v/I");  
    _algo_tree->Branch("_variance_vals_v",&_variance_vals_v,"variance_vals_v/I");  
//Y plane
    _algo_tree->Branch("_delb12y",&_delb12y,"delb12y/I");
    _algo_tree->Branch("_delb13y",&_delb13y,"delb13y/I");
    _algo_tree->Branch("_delv12y",&_delv12y,"delv12y/I");   
    _algo_tree->Branch("_delv13y",&_delv13y,"delv13y/I");
    _algo_tree->Branch("_baseline_vals_y",&_baseline_vals_y,"baseline_vals_y/I");  
    _algo_tree->Branch("_variance_vals_y",&_variance_vals_y,"variance_vals_y/I");  
        
    _algo_tree->Branch("_channel",&_channel,"channel/I");
    _algo_tree->Branch("_postHuffmanwords",&_postHuffmanwords,"postHuffmanwords/I");
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
    
    _channel = ch;//Anya edit
    // iterator to the beginning and end of the waveform
    _begin = waveform.begin();
    _end   = waveform.end();
    
    std::vector<short> outputwf;
    _baselines.clear();
    _variances.clear();
    
    unsigned int _baseline[3];
    unsigned int _variance[3];
    _baseline[0] = std::numeric_limits<unsigned int>::max();
    _baseline[1] = std::numeric_limits<unsigned int>::max();
    _baseline[2] = std::numeric_limits<unsigned int>::max();
    _variance[0] = std::numeric_limits<unsigned int>::max();
    _variance[1] = std::numeric_limits<unsigned int>::max();
    _variance[2] = std::numeric_limits<unsigned int>::max();
    
    // Tick @ which waveform starts
    int start = 0;

    // start & end tick for each Region Of Interest (ROI) saved
    tick s;
    tick e;
    std::pair<tick,tick> thisRange; 
    
    _pl = mode;

    for (size_t n = 0; n < waveform.size(); n++) {
      _thisTick = _begin + n;
      size_t bob = 1;
      _nextTick = _begin + n + bob;

      double thisTick = *_thisTick;
      double nextTick = *_nextTick;


      if (n % (_block - 1) == 0){

	if ( (n + 1 - _block) == 0){
          int baseline = 0;
          int var      = 0;
          int diff     = 0;
          tick t = _thisTick - _block +1;
          int mm = 0;
          for (; t < _thisTick + 1; t++){
            baseline += *t;
            mm += 1;
          }
          baseline /= _block;
          t = _thisTick - _block + 1;
          for(; t < _thisTick + 1; t++){
            diff = ( (*t) - baseline ) * ( (*t) - baseline );
            if (diff < 4095) var += diff;
            else var += 4095;
          }
          var = var >> 6;
          _baseline[1] = baseline;
          _variance[1] = var;
          
	  baseline = 0;
	  var      = 0;
	  diff     = 0;
	  t = _thisTick + 1;
	  for (; t < _thisTick + _block + 1; t++)
	    baseline += *t;
	  baseline = baseline >> 6;
	  t = _thisTick + 1;
	  int nn = 0;
	  for (; t < _thisTick + _block + 1; t++){
	    diff = ( (*t) - baseline ) * ( (*t) - baseline );
	    if (diff < 4095) var += diff;
	    else var += 4095;
	    nn += 1;
	  }
	  var = var >> 6;
	  _baseline[2] = baseline;
	  _variance[2] = var;

//Anya variables
      _baseline_vals = _baseline[0];
      _baseline_vals = _baseline[0];
      _variance_vals = _variance[1];
      _variance_vals = _variance[1];

	}// 1st block updating

	// always compute baseline and variance for next block, if it exists
	if ( (n + _block) < waveform.size() ){
	  int baseline = 0;
	  int var      = 0;
	  int diff     = 0;
	  tick t = _thisTick + 1;
	  for (; t < _thisTick + _block + 1; t++)
	    baseline += *t;
	  baseline = baseline >> 6;
	  t = _thisTick + 1;
	  for (; t < _thisTick + _block + 1; t++){
	    diff = ( (*t) - baseline ) * ( (*t) - baseline );
	    if (diff < 4095) var += diff;
	    else var += 4095;
	  }
	  var = var >> 6;

	  // update baselines and variances

	  // shift baselines and variances by 1 to accomodate
	  // for the new value from the last block
	  _baseline[0] = _baseline[1];
	  _variance[0] = _variance[1];
	  _baseline[1] = _baseline[2];
	  _variance[1] = _variance[2];
	  // add the newly calculated value
	  // to the last element
	  _baseline[2] = baseline;
	  _variance[2] = var;

     
          //Anya edit: variables delta b_12, delta b_13, delta v_12, delta v_13
          
          _delb12 = _baseline[1] - _baseline[0];//b_2-b_1
          _delb13 = baseline - _baseline[0];//b_3-b_1
          _delv12 = _variance[1] - _variance[0];//v_2-v_1   
          _delv13 = var - _variance[0];//v_3-v_1
          
          _baseline_vals = baseline;
          _variance_vals = var;
          
          //U plane
          if (_pl==0){
          _baseline_vals_u = baseline;
          _variance_vals_u = var;
          
          _delb12u = _baseline[1] - _baseline[0];//b_2-b_1
          _delb13u = baseline - _baseline[0];//b_3-b_1
          _delv12u = _variance[1] - _variance[0];//v_2-v_1   
          _delv13u = var - _variance[0];//v_3-v_1
          
          }
          //V plane
          if (_pl==1){
          _baseline_vals_v = baseline;
          _variance_vals_v = var;
          
          _delb12v = _baseline[1] - _baseline[0];//b_2-b_1
          _delb13v = baseline - _baseline[0];//b_3-b_1
          _delv12v = _variance[1] - _variance[0];//v_2-v_1   
          _delv13v = var - _variance[0];//v_3-v_1
           }
          //Y plane
          if (_pl==2) {
          _baseline_vals_y = baseline;
          _variance_vals_y = var;
          
          _delb12y = _baseline[1] - _baseline[0];//b_2-b_1
          _delb13y = baseline - _baseline[0];//b_3-b_1
          _delv12y = _variance[1] - _variance[0];//v_2-v_1   
          _delv13y = var - _variance[0];//v_3-v_1
          }

//std::vector<double> ax;
//std::vector<double> entries;
//std::vector<int> channels;
//for (int z =0; z<8256; z++){
//	if (_ch=z){
// 	entries.pushback(variance);
//	channels.pushback(z);
//	}
//ax.pushback(entries);
//}




        }// if we are updating the NEXT block

         if (_debug){
          std::cout << "Baseline. Block 1: " << _baseline[0] << "\tBlock 2: " << _baseline[1] << "\tBlock 3: " << _baseline[2] << std::endl;
	  std::cout << "Variance. Block 1: " << _variance[0] << "\tBlock 2: " << _variance[1] << "\tBlock 3: " << _variance[2] << std::endl;


	}


	// Determine if the 3 blocks are quiet enough to update the baseline
	if ( ( (_baseline[2] - _baseline[1]) * (_baseline[2] - _baseline[1]) < _deltaB ) && 
	     ( (_baseline[2] - _baseline[0]) * (_baseline[2] - _baseline[0]) < _deltaB ) && 
	     ( (_baseline[1] - _baseline[0]) * (_baseline[1] - _baseline[0]) < _deltaB ) &&
	     ( (_variance[2] - _variance[1]) * (_variance[2] - _variance[1]) < _deltaV ) &&
	     ( (_variance[2] - _variance[0]) * (_variance[2] - _variance[0]) < _deltaV ) &&
	     ( (_variance[1] - _variance[0]) * (_variance[1] - _variance[0]) < _deltaV ) ){
	  _baselineMap[ch] = _baseline[1];
	  if (_debug) std::cout << "Baseline updated to value " << _baselineMap[ch] << std::endl;
	}

	_algo_tree->Fill();
      }// if we hit the end of a new block

      _v1 = _variance[0];
      _v2 = _variance[1];
      _v3 = _variance[2];
      _b1 = _baseline[0];
      _b2 = _baseline[1];
      _b3 = _baseline[2];








      if ( (_baselineMap.find(ch) != _baselineMap.end()) ){
	
	double base = _baselineMap[ch];
	
	// reset maxima
	_max = 0;

	// Then go through the 3 blocks again trying to find a waveform to save
	
	// save will keep track of tick at which waveform goes above threshold
	// == 0 if not -> use as logic method to decide if to push back or not
	_save = 0;
	// also keep track of each new sub-waveform to push back to output waveform
	outputwf.clear();
	
	// loop over central block to search for above-threshold regions
	

	//for (t = _thisTick + _block; t < _thisTick + 2 * _block; t++){
	
	tick t = _thisTick;
	
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

          for (size_t c = 0; c < outputwf.size(); c++){
		this_Tick = _begin + c;
		size_t one = 1;
		next_Tick = _begin + c + one;

		double thisTick = *this_Tick;
		double nextTick = *next_Tick;

		//ApplyHuffman(_this_Tick, _next_Tick);
	       int postHuffwords = 0; //words are 16 bits long, with 4 bit headers             	
               int postHuffmanbits = 0;                                                        
               const int availablewordbits = 15;                                               
                                                                                           
               if ( nextTick - thisTick == 0 ){ 
                if ( postHuffmanbits + 1 < availablewordbits) { postHuffmanbits += 1; }   
                else {postHuffwords += 1; postHuffmanbits = 1; }}                         
               else if ( nextTick - thisTick ==-1 ){                                      
                 if (postHuffmanbits + 2 < availablewordbits ) { postHuffmanbits += 2; }  
                 else{ postHuffwords += 1; postHuffmanbits = 2; }}                        
               else if ( nextTick - thisTick == 1 ){                                      
                if (postHuffmanbits + 3 < availablewordbits ) { postHuffmanbits += 3; }   
                 else{ postHuffwords += 1; postHuffmanbits = 3;  }}                       
               else if ( nextTick - thisTick ==-2 ){                                      
                if (postHuffmanbits + 4 < availablewordbits ) { postHuffmanbits += 4; }   
                 else{ postHuffwords += 1; postHuffmanbits = 4;  }}                       
               else if ( nextTick - thisTick == 2 ){                                      
                if (postHuffmanbits + 5 < availablewordbits ) { postHuffmanbits += 5; }   
                 else{ postHuffwords += 1; postHuffmanbits = 5;  }}                       
               else if ( nextTick - thisTick ==-3 ){                                      
                if (postHuffmanbits + 6 < availablewordbits ) { postHuffmanbits += 6; }   
                 else{ postHuffwords += 1; postHuffmanbits = 6;  }}                       
               else if ( nextTick - thisTick == 3 ){                                      
                if (postHuffmanbits + 7 < availablewordbits ) { postHuffmanbits += 7; }   
                 else{ postHuffwords += 1; postHuffmanbits = 7;  }}                       

	//       _save = 0;

          }


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

    }// for all ticks

    return;
  }
//Anya function: Huffman compression simulation compression factor
/*
  int CompressionAlgosncompress::ApplyHuffman(double thisTick,double nextTick){
       int postHuffwords = 0; //words are 16 bits long, with 4 bit headers
       int postHuffmanbits = 0;                                                        
       const int availablewordbits = 15;                                               
                                                                                   
       if ( nextTick - thisTick == 0 ){ 
        if ( postHuffmanbits + 1 < availablewordbits) { postHuffmanbits += 1; }   
        else {postHuffwords += 1; postHuffmanbits = 1; }}                         
       else if ( nextTick - thisTick ==-1 ){                                      
         if (postHuffmanbits + 2 < availablewordbits ) { postHuffmanbits += 2; }  
         else{ postHuffwords += 1; postHuffmanbits = 2; }}                        
       else if ( nextTick - thisTick == 1 ){                                      
        if (postHuffmanbits + 3 < availablewordbits ) { postHuffmanbits += 3; }   
         else{ postHuffwords += 1; postHuffmanbits = 3;  }}                       
       else if ( nextTick - thisTick ==-2 ){                                      
        if (postHuffmanbits + 4 < availablewordbits ) { postHuffmanbits += 4; }   
         else{ postHuffwords += 1; postHuffmanbits = 4;  }}                       
       else if ( nextTick - thisTick == 2 ){                                      
        if (postHuffmanbits + 5 < availablewordbits ) { postHuffmanbits += 5; }   
         else{ postHuffwords += 1; postHuffmanbits = 5;  }}                       
       else if ( nextTick - thisTick ==-3 ){                                      
        if (postHuffmanbits + 6 < availablewordbits ) { postHuffmanbits += 6; }   
         else{ postHuffwords += 1; postHuffmanbits = 6;  }}                       
       else if ( nextTick - thisTick == 3 ){                                      
        if (postHuffmanbits + 7 < availablewordbits ) { postHuffmanbits += 7; }   
         else{ postHuffwords += 1; postHuffmanbits = 7;  }}                       
       postHuffwords = _postHuffmanwords; 

  return postHuffwords;
  } */

  bool CompressionAlgosncompress::PassThreshold(double thisADC, double base){

    //BEGIN ANYA EDIT

    if (_pol[_pl] == 0){ //unipolar setting set at command line

        //if positive threshold
	  if (_thresh[_pl] >= 0){
          if (thisADC > base + _thresh[_pl])
    	return true;
       }

	// if negative threshold
        else{
          if (thisADC < base + _thresh[_pl])
    	return true;
        }

	  }

    else { //bipolar setting set at command line
      if  (thisADC >= base + std::abs(_thresh[_pl])) {
	    return true;
	  }

      if (thisADC <= base - std::abs(_thresh[_pl])) {
	      return true;
	    }
	  }
    //END ANYA EDIT
    
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

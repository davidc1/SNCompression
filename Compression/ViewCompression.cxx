#ifndef VIEWCOMPRESSION_CXX
#define VIEWCOMPRESSION_CXX

#include "ViewCompression.h"

namespace larlite {

  bool ViewCompression::initialize() {

    _evtNum = 0;

    return true;
  }
  
  bool ViewCompression::analyze(storage_manager* storage) {

    //reset WF counter
    _currentWF = 0;

    // If no compression algorithm has been defined, skip
    if ( _compress_algo == 0 ){
      print(msg::kERROR,__FUNCTION__,"Compression Algorithm Not Set! Exiting");
      return false;
    }

    // Otherwise Get RawDigits and execute compression
    auto event_wf = storage->get_data<event_rawdigit>("daq");
    // If raw_digits object is empty -> exit
    if(!event_wf) {
      print(msg::kERROR,__FUNCTION__,"Data storage did not find associated waveforms!");
      return false;
    }

    _numWFs = event_wf->size();

    _current_event_wf = event_wf;

    _evtNum += 1;
    
    return true;
  }
  
  bool ViewCompression::finalize() {
    
    return true;
  }
  
  
  void ViewCompression::processWF(){

    
    //get rawdigit
    larlite::rawdigit* tpc_data = (&(_current_event_wf->at(_currentWF)));      

    // Figure out channel's plane:
    // used because different planes will have different "buffers"
    UInt_t ch = tpc_data->Channel();
    int pl = larutil::Geometry::GetME()->ChannelToPlane(ch);

    // reset compression
    _compress_algo->Reset();
    //finally, apply compression..
    const std::vector<short> &ADCwaveform = tpc_data->ADCs();
    _compress_algo->ApplyCompression(ADCwaveform,pl,ch);
    auto const ranges = _compress_algo->GetOutputRanges();
    
    _NumOutWFs = ranges.size();

    // calculate an approximate baseline
    _base = 0;
    if (_baseline){
      for (size_t i=0; i < 200; i++)
	_base += ADCwaveform[i];
      _base /= 200.;
    }
    //clear histograms
    ClearHistograms();
    //now fill histograms
    //FillHistograms(ADCwaveform, compressOutput, outTimes, ch, pl);
    FillHistograms(ADCwaveform, ranges, ch, pl);
    //fill baseline & variance histograms
    FillBaseVarHistos(_compress_algo->GetBaselines(),_compress_algo->GetVariances(),ch,pl);

    _currentWF += 1;
    
    return;
  }
  

  void ViewCompression::FillHistograms(const std::vector<short> ADCwaveform,
				       const std::vector<std::pair< compress::tick, compress::tick> > ranges,
				       UShort_t ch,
				       UChar_t pl){
    
    _hInWF = new TH1D("hInWF", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; ADCs",_evtNum, pl, ch),
		      ADCwaveform.size(), 0, ADCwaveform.size());

    _hOutWF = new TH1D("hOutWF", Form("Event %i - Pl %i - Ch %i - Output WF; Time Tick; ADCs",_evtNum, pl, ch),
		       ADCwaveform.size(), 0, ADCwaveform.size());

    _hInWF->SetTitleOffset(0.8,"X");
    _hOutWF->SetTitleOffset(0.8,"X");
    
    for (size_t n=0; n < ADCwaveform.size(); n++)
      _hInWF->SetBinContent(n+1, ADCwaveform.at(n)-_base);

    //measure a baseline to place a temporary holder in output histogram
    double baseline = 0.;
    for (int tt=0; tt < 10; tt++)
      baseline += ADCwaveform.at(tt);
    baseline /= 10.;
    int base = int(baseline);
    for (size_t m=0; m < ADCwaveform.size(); m++)
      _hOutWF->SetBinContent(m+1, base-_base);

    // iterator to beginning of input waveform
    const compress::tick begin = _compress_algo->GetInputBegin();
    
    for (size_t j=0; j < ranges.size(); j++){
      std::cout << "Range: [" <<  int(ranges.at(j).first-begin) << ", "
		<< int(ranges.at(j).second-begin)
		<< "]" << std::endl;
      compress::tick t;
      for (t = ranges.at(j).first; t < ranges.at(j).second; t++)
	_hOutWF->SetBinContent( int(t-begin), *t-_base);
    }
    _hInWF->SetAxisRange(_hInWF->GetMinimum(), _hInWF->GetMaximum(), "Y");
    _hOutWF->SetAxisRange(_hInWF->GetMinimum(), _hInWF->GetMaximum(), "Y");
    
    return;
  }


  void ViewCompression::FillBaseVarHistos(const std::vector<double>& base,
					  const std::vector<double>& var,
					  UShort_t ch,
					  UChar_t pl){
    
    // block size is 64
    int block = 64;
    int nblocks = base.size();

    _hInBase = new TH1D("hInBase", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; Baseline",_evtNum, pl, ch),
		      block*nblocks, 0, block*nblocks);

    _hInVar = new TH1D("hOutVar", Form("Event %i - Pl %i - Ch %i - Output WF; Time Tick; Variance",_evtNum, pl, ch),
		       block*nblocks, 0, block*nblocks);

    _hInWF->SetTitleOffset(0.8,"X");
    _hOutWF->SetTitleOffset(0.8,"X");
    
    for (size_t n=0; n < nblocks; n++){
      for (size_t i=0; i < block; i++){
	_hInBase->SetBinContent(n*block+i, base[n]-_base);
	_hInVar->SetBinContent(n*block+i, var[n]);
      }
    }

    return;
  }
  
}
#endif

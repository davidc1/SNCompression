#ifndef VIEWCOMPRESSION_CXX
#define VIEWCOMPRESSION_CXX

#include "ViewCompression.h"

namespace compress {

  ViewCompression::ViewCompression()
    : _c1(nullptr)
    , _p1(nullptr)
    , _hInWF(nullptr)
    , _hInBase(nullptr)
    , _hInVar(nullptr)
    , _hOutWF(nullptr)
    , _hIDEs(nullptr)
  {
    _baseline = false;
  };

  void ViewCompression::FillHistograms(const std::pair<compress::tick,compress::tick>& range,
				       const std::vector<std::pair< compress::tick, compress::tick> >& ranges,
				       int evt, UShort_t ch, UChar_t pl){

    _evtNum = evt;
    _ch = ch;
    _pl = pl;
    
    _NumOutWFs = ranges.size();

    // delete any histograms if they alerady existed
    if (_hInWF) { delete _hInWF; }
    if (_hOutWF) { delete _hOutWF; }

    if (pl == 2) { _base = 400; }
    else { _base = 2048; }
    if (_baseline == false) { _base = 0; }


    _hInWF = new TH1D("hInWF", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; ADCs",evt, pl, ch),
		      std::distance(range.first,range.second), 0, std::distance(range.first,range.second));

    _hOutWF = new TH1D("hOutWF", Form("Event %i - Pl %i - Ch %i - Output WF; Time Tick; ADCs",evt, pl, ch),
		       std::distance(range.first,range.second), 0, std::distance(range.first,range.second));

    _in_ADC_v.clear();
    _out_ADC_v.clear();

    _hInWF->SetTitleOffset(0.8,"X");
    _hOutWF->SetTitleOffset(0.8,"X");
    
    for (tick t = range.first; t < range.second; t++){
      _hInWF->SetBinContent(std::distance(range.first,t)+1, *t-_base);
      _in_ADC_v.push_back(*t-_base);
      _out_ADC_v.push_back(0);
      //_hOutWF->SetBinContent(std::distance(range.first,t)+1, 0.);
    }

    
    for (size_t j=0; j < ranges.size(); j++){
      std::cout << "Range: [" <<  std::distance(range.first,ranges.at(j).first) << ", "
      		<< std::distance(range.first,ranges.at(j).second)
      		<< "]" << std::endl;
      tick t;
      for (t = ranges.at(j).first; t < ranges.at(j).second; t++){
	_hOutWF->SetBinContent( std::distance(range.first,t), *t-_base);
	_out_ADC_v[std::distance(range.first,t)] = (*t-_base);
      }
    }
    _hInWF->SetAxisRange(_hInWF->GetMinimum(), _hInWF->GetMaximum(), "Y");
    _hOutWF->SetAxisRange(_hInWF->GetMinimum(), _hInWF->GetMaximum(), "Y");
    
    return;
  }


  void ViewCompression::ResetIDEs(int evt, UShort_t ch, UChar_t pl, size_t ADClen)
  {

    if (_hIDEs) { delete _hIDEs; }

    _hIDEs = new TH1D("hIDEs", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; ADCs",evt, pl, ch),
		      ADClen, 0, ADClen);

    _in_IDE_v.clear();
    
    // first fill all with zeros
    for (size_t i=0; i < ADClen; i++){
      _hIDEs->SetBinContent(i+1,0);
      _in_IDE_v.push_back(0);
    }

    return;
  }

  void ViewCompression::FillIDEs(const std::vector<std::pair<unsigned short, double> >& IDEs,
				 int evt, UShort_t ch, UChar_t pl, size_t ADClen)
  {

    if (_hIDEs) { delete _hIDEs; }

    _hIDEs = new TH1D("hIDEs", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; ADCs",evt, pl, ch),
		      ADClen, 0, ADClen);

    _in_IDE_v.clear();
    
    // first fill all with zeros
    for (size_t i=0; i < ADClen; i++){
      _hIDEs->SetBinContent(i+1,0);
      _in_IDE_v.push_back(0);
    }
    
    // now loop through IDEs filling appropriaate values
      for (auto &ide : IDEs){
      _hIDEs->SetBinContent(ide.first,ide.second);
      _in_IDE_v[ide.first] = ide.second;
      }
    
    return;
  }

  void ViewCompression::FillBaseVarHistos(const std::vector<double>& base,
					  const std::vector<double>& var,
					  int evt, UShort_t ch, UChar_t pl)
  {
    
    // delete histograms if they existed already
    if (_hInBase) { delete _hInBase; }
    if (_hInVar) { delete _hInVar; }
    
    // block size is 64
    size_t block = 64;
    size_t nblocks = base.size();

    _hInBase = new TH1D("hInBase", Form("Event %i - Pl %i - Ch %i - Input WF; Time Tick; Baseline",evt, pl, ch),
		      block*nblocks, 0, block*nblocks);

    _hInVar = new TH1D("hOutVar", Form("Event %i - Pl %i - Ch %i - Output WF; Time Tick; Variance",evt, pl, ch),
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

#ifndef COMPRESSIONSTUDYBASELINE_CXX
#define COMPRESSIONSTUDYBASELINE_CXX

#include "CompressionStudyBaseline.h"

namespace compress {


  CompressionStudyBaseline::CompressionStudyBaseline()
    : CompressionStudyBase()
    , _base_tree(nullptr)
    , _model(nullptr)
  {
    
    if (_base_tree) { delete _base_tree; }
    _base_tree = new TTree("base_tree","Buffer Study Tree");
    _base_tree->Branch("_base_pre",&_base_pre,"base_pre/D");
    _base_tree->Branch("_base_post",&_base_post,"base_post/D");
    _base_tree->Branch("_slope_pre",&_slope_pre,"slope_pre/D");
    _base_tree->Branch("_slope_post",&_slope_post,"slope_post/D");
    _base_tree->Branch("_pl",&_pl,"pl/I");

    PrepareLinearModel();

    return;
  }

  void CompressionStudyBaseline::StudyCompression(const std::vector<short> &in,
					      const std::vector<std::pair< compress::tick, compress::tick> > &ranges,
					      const int pl)
  {

    _pl = pl;

    _begin = in.begin();

    // function to calculate baseline and slope values before/after pulse region
    for (auto const& range : ranges)
      EvaluateBaseline(range);
    return;
  }


  void CompressionStudyBaseline::EvaluateBaseline(const std::pair<tick,tick>& range){

    _base_pre = 0;
    _base_post = 0;
    _slope_pre = 0;
    _slope_post = 0;
    
    const int n=4;
    float DataFront[n] = {0,0,0,0};
    float DataBack[n]  = {0,0,0,0};
    float Tick[n] = {1,2,3,4};
    float Errors[n] = {0.5,0.5,0.5,0.5};
    float TickErr[n] = {0.,0.,0.,0.};
    
    // calculate baseline pre and post
    tick t;
    int counter = 0;
    for (t = range.first; t != range.first+4; t++){
      if (_verbose) { std::cout << " T: " << counter << " ADC: " << *t << "\tdist: " << std::distance(_begin,t) << std::endl; }
      _base_pre += *t;
      DataFront[counter] = *t;
      counter += 1;
    }
    _base_pre /= 4.;
    if (_verbose) { std::cout << std::endl; }

    counter = 0;
    for (t = range.second-2; t != range.second-6; t--){
      if (_verbose) { std::cout << " T: " << counter << " ADC: " << *t << "\tdist: " << std::distance(_begin,t) << std::endl; }
      _base_post += *t;
      DataBack[counter] = *t;
      counter += 1;
    }
    _base_post /= 4.;
    if (_verbose) { std::cout << std::endl; }


    // calculate slope pre and post
    TGraph *grPre  = new TGraphErrors(n,Tick,DataFront,TickErr,Errors);
    TGraph *grPost = new TGraphErrors(n,Tick,DataBack,TickErr,Errors);
    
    grPre->Fit("Model","QN");
    _slope_pre = _model->GetParameter(1);    
    if (_verbose) { std::cout << "\t\tBaseline pre: " << _base_pre << "\t\tSlope pre: " << _slope_pre << std::endl; }
    grPost->Fit("Model","QN");
    _slope_post = _model->GetParameter(1);    
    if (_verbose) { std::cout << "\t\tBaseline post: " << _base_post << "\t\tSlope post: " << _slope_post << std::endl; }
    
    delete grPre;
    delete grPost;

    _base_tree->Fill();

    return;
  }


  void CompressionStudyBaseline::PrepareLinearModel(){
    
    if (_model) delete _model;
    _model = new TF1("Model","[0]+[1]*x",1,4);
    _model->SetParName(0,"const");
    _model->SetParName(1,"slope");
    return;
  }
  
  
  void CompressionStudyBaseline::EndProcess(TFile *fout){

    if (fout){
      if (_base_tree)
	_base_tree->Write();
    }
    return;
  }
  
}

#endif

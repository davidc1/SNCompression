#ifndef STUDYCOMPRESSIONBUFFERSIZE_CXX
#define STUDYCOMPRESSIONBUFFERSIZE_CXX

#include "StudyCompressionBufferSize.h"

namespace larlite {


  StudyCompressionBufferSize::StudyCompressionBufferSize()
    : _base_tree(nullptr)
    , _compress_algo(nullptr)
    , Model(nullptr)  
  {
    _name="StudyCompressionBufferSize";
    _fout=0;
    _compress_algo = 0; 
  };

  bool StudyCompressionBufferSize::initialize() {
    

    if (!_base_tree)
      _base_tree = new TTree("_base_tree","Baseline Tree");
    _base_tree->Branch("_base_pre",&_base_pre,"base_pre/D");
    _base_tree->Branch("_base_post",&_base_post,"base_post/D");
    _base_tree->Branch("_slope_pre",&_slope_pre,"slope_pre/D");
    _base_tree->Branch("_slope_post",&_slope_post,"slope_post/D");
    _base_tree->Branch("_baseChan",&_baseChan,"baseChan/D");
    _base_tree->Branch("_varChan",&_varChan,"varChan/D");
    _base_tree->Branch("_ch",&_ch,"ch/I");
    _base_tree->Branch("_sygType",&_sygType,"sygType/I");
    _base_tree->Branch("_plane",&_plane,"plane/I");


    _evt = 0;

    PrepareLinearModel();

    return true;
  }
  
  bool StudyCompressionBufferSize::analyze(storage_manager* storage) {

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

    // clear place-holder for new, compressed, waveforms
    _out_event_wf.clear();

    // reset variables that hold compression factor
    _inTicks  = 0;
    _outTicks = 0;
    
    // Loop over all waveforms
    for (size_t i=0; i<event_wf->size(); i++){
      
      //get tpc_data
      larlite::rawdigit* tpc_data = (&(event_wf->at(i)));      
      //Check for empty waveforms!
      if(tpc_data->ADCs().size()<1){
	print(msg::kERROR,__FUNCTION__,
	      Form("Found 0-length waveform: Event %d ... Ch. %d",event_wf->event_id(),tpc_data->Channel()));
	continue;
      }//if wf size < 1

      // Figure out channel's plane:
      // used because different planes will have different "buffers"
      UInt_t ch = tpc_data->Channel();
      int pl = larutil::Geometry::GetME()->ChannelToPlane(ch);

      _plane = pl;
      _ch = ch;

      // finally, apply compression:
      // *-------------------------*
      // 1) Convert tpc_data object to just the vector of shorts which make up the ADC ticks
      const std::vector<short> ADCwaveform = tpc_data->ADCs();
      // Calculate total baseline for channel
      _baseChan = 0;
      _varChan = 0;
      for (size_t b=0; b < 200; b++)
	_baseChan += ADCwaveform[b];
      _baseChan/=200.;
      for (size_t c=0; c < 200; c++)
	_varChan += (ADCwaveform[c]-_baseChan)*(ADCwaveform[c]-_baseChan);
      _varChan = sqrt(_varChan/200.);

      // 2) Now apply the compression algorithm. _compress_algo is an instance of CompressionAlgoBase
      _compress_algo->ApplyCompression(ADCwaveform,pl,ch);
      // 3) Retrieve the output waveforms produced during the compression
      auto const ranges = _compress_algo->GetOutputRanges();
      // Now, for each output waveform, calcualte pre & post baseline and fill TTree with info
      for (auto &range : ranges)
	StudyBaseline(range);
      // 6) clear _InWF and _OutWF from compression algo object -> resetting algorithm for next time it is called
      _compress_algo->Reset();
      
    }//for all waveforms

    _evt += 1;
    
    //now take new WFs and place in event_wf vector
    event_wf->clear();
    for (size_t m=0; m < _out_event_wf.size(); m++)
      event_wf->push_back(_out_event_wf.at(m));
    return true;
  }

  bool StudyCompressionBufferSize::finalize() {

    _compress_algo->EndProcess(_fout);
    _base_tree->Write();

    return true;
  }

  void StudyCompressionBufferSize::StudyBaseline(const std::pair< compress::tick, compress::tick> &range){

    size_t l = range.second-range.first;

    if (l < 6)
      return;

    float t0 = *(range.first);
    float t1 = *(range.first+1);
    float t2 = *(range.first+2);
    float t3 = *(range.second-2);
    float t4 = *(range.second-1);
    float t5 = *(range.second);

    _base_pre   = (t0+t1+t2)/3.;
    _base_post  = (t3+t4+t5)/3.;
    _slope_pre  = 0;
    _slope_post = 0;
    
    /*
    const int n=3;
    float DataFront[n] = {t0,t1,t2};
    float DataBack[n] = {t3,t4,t5};
    float Tick[n] = {1,2,3};
    float Errors[n] = {0.5,0.5,0.5};
    float TickErr[n] = {0,0,0};

    TGraph *grPre  = new TGraphErrors(n,Tick,DataFront,TickErr,Errors);
    TGraph *grPost = new TGraphErrors(n,Tick,DataBack,TickErr,Errors);

    grPre->Fit("Model","QN");
    _slope_pre = Model->GetParameter(1);    
    grPost->Fit("Model","QN");
    _slope_post = Model->GetParameter(1);    
    
    delete grPre;
    delete grPost;
    */
    _base_tree->Fill();

    return;
  }


  void StudyCompressionBufferSize::PrepareLinearModel(){

    if (Model) delete Model;
    Model = new TF1("Model","[0]+[1]*x",1,3);
    Model->SetParName(0,"const");
    Model->SetParName(1,"slope");
    return;
  }

}
#endif

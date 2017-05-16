#ifndef LARLITE_SNHITRECO_CXX
#define LARLITE_SNHITRECO_CXX

#include "SNHitReco.h"

#include "DataFormat/wire.h"
#include "DataFormat/hit.h"

#include "LArUtil/Geometry.h"

namespace larlite {

  bool SNHitReco::initialize() {

    return true;
  }
  
  bool SNHitReco::analyze(storage_manager* storage) {

    if ( (_wire_producer == "") || (_hit_producer == "") ) {
      std::cout << "did not specify wire AND hit producers...quit." << std::endl;
      return true;
    }

    // read in Wires
    auto const ev_wf   = storage->get_data<event_wire>(_wire_producer);
    // create Hits
    auto const ev_hits = storage->get_data<event_hit>(_hit_producer);

    // set event ID through storage manager
    storage->set_id(storage->get_data<event_wire>(_wire_producer)->run(),
		    storage->get_data<event_wire>(_wire_producer)->subrun(),
		    storage->get_data<event_wire>(_wire_producer)->event_id());

    // loop over waveforms
    for (size_t i=0; i < ev_wf->size(); i++){

      auto const wf = ev_wf->at(i);

      //std::cout << "CHANNEL " << wf.Channel() << std::endl;

      // get vector of ROIs
      auto const& ROIs = wf.SignalROI();
      
      for (auto const& ROI : ROIs.get_ranges()){

	// each ROI is a vector of floats containing the ADC ticks
	// recorded by the zero-suppression algorithm

	// for each ROI we will identify the value of the first tick
	// as the baseline

	// the peak-amplitude will be determined by the largest ADC value
	// after baseline subtraction

	// the hit integral will be calculated as the sum over the entire
	// ROI of the ADC value - the baseline.

	// note that this procedure has several flaws:

	// 1) no concept of negative polarity is accounted for. This procedure
	// will work correctly for positive unipolar pulses such as on the
	// collection-plane, but may perform poorly for induction-plane signals

	// 2) the determination of the baseline is rough (subject to tick-by-tick
	// noise fluctuations) and only as good as the ROI selection itself:
	// if the ROI does not start during a quiet baseline region of the waveform
	// then the baseline determination will be incorrect.

	//auto baseline = ROI.at(0);
        size_t last = ROI.size()-1;
        auto baseline = (ROI.at(0)+ROI.at((int)last))/2;

	double amplitude = 0;
	double area      = 0;
	size_t peak      = 0;
	
	for (size_t t=0; t < ROI.size(); t++) {

	  auto adc = ROI[t];
	  auto val = adc - baseline;

	  if (val > 0)  area += val;
	  if (val > amplitude) { amplitude = val; peak = t; }

	}// for all ADC ticks in waveform

	if (amplitude <= 0) continue;
	if (amplitude < _min_ampl) continue;

	// grab the start time of the ROI w.r.t. the beginning of the 1st frame in the
	// event.
	auto ROIstart = ROI.begin_index();

	//std::cout << "\t\t Hit start time = " << ROIstart << " @ wire " << wf.Channel()
	//	  << " @ amplitude " << amplitude <<  std::endl;

	// we now have all the information needed to create a new hit
	larlite::hit hit;
	hit.set_time_range(peak + ROIstart - 27, peak + ROIstart + 28);
	hit.set_time_peak(peak + ROIstart,0.);
	hit.set_time_rms(0.4 * area / amplitude);
	hit.set_amplitude(amplitude,0.);
	hit.set_sumq(area);
	hit.set_integral(area,0.);

	// set wire information
	auto ch         = wf.Channel();
	auto const view = larutil::Geometry::GetME()->View(ch);
	auto const pl   = larutil::Geometry::GetME()->ChannelToPlane(ch);
	auto const sigt = larutil::Geometry::GetME()->SignalType(ch);
	auto wire       = larutil::Geometry::GetME()->ChannelToWireID(ch);
	
	hit.set_channel(ch);
	hit.set_view(view);
	hit.set_signal_type(sigt);
	hit.set_wire(wire);

	ev_hits->push_back(hit);
	
      }// for all ROIs in the wire

    }// loop thrugh wires

    return true;
  }

  bool SNHitReco::finalize() {

    return true;
  }

}
#endif

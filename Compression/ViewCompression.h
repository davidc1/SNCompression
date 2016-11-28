/**
 * \file ViewCompression.h
 *
 * \ingroup Analysis
 *
 * \brief Class def header for a class ViewCompression
 *
 * @author David Caratelli
 */

/** \addtogroup Analysis 
    
    @{*/
 
#ifndef VIEWCOMPRESSION_H
#define VIEWCOMPRESSION_H

#include <TH1D.h>
#include "CompressionAlgoBase.h"
#include <TH1D.h>
#include <TCanvas.h>
#include <TPad.h>

namespace compress {
  /**
     \class ViewCompression
     User custom analysis class made by David Caratelli
  */
  class ViewCompression {
    
  public:
    
    /// Default constructor
    ViewCompression();
    
    /// Default destructor
    virtual ~ViewCompression(){};
    
    /// PrepareCanvas
    void PrepareCanvas() { _c1 = new TCanvas("c1", "canvas", 900, 900); _c1->Divide(1,2); }
    
    void UpdateCanvas() { _c1->cd(1); _hInWF->Draw(); _c1->cd(2); _hOutWF->Draw(); }

    /// Set boolean whether to use baseline or not 
    void suppressBaseline(bool on) { _baseline = on; } 
    
    /// Get Number of output waveforms
    int GetNumOutWFs() { return _NumOutWFs; }
    
    //// Clear Histograms
    void ClearHistograms() { delete _hInWF; delete _hOutWF; delete _hInBase; delete _hInVar; delete _hIDEs; }
    
    /// Fill Histograms with new and old waveforms
    void FillHistograms(const std::pair<compress::tick,compress::tick>& range,
			const std::vector<std::pair< compress::tick, compress::tick> >& ranges,
			int evt, UShort_t ch, UChar_t pl);

    /// Fill IDE vector
    void FillIDEs(const std::vector<std::pair<unsigned short, double> >& IDEs,
		  int evt, UShort_t ch, UChar_t pl, size_t ADClen);

    /// reset IDE histogram
    void ResetIDEs(int evt, UShort_t ch, UChar_t pl, size_t ADClen);

    void FillBaseVarHistos(const std::vector<double>& base,
			   const std::vector<double>& var,
			   int evt, UShort_t ch, UChar_t pl);

    /// Get Histograms for python script
    const TH1D* GetHistos(int which) const {
      if (which == 1)
	return _hInWF;
      else if (which == 2)
	return _hOutWF;
      else{
	std::cout << "not valid input...returning _hInWF" << std::endl;
	return _hInWF;
      }

    }

    /// Get vector of ADCs
    const std::vector<double> GetInADCs() const {
      return _in_ADC_v;
    }
    const std::vector<std::vector<double> > GetOutADCs() const {
      return _out_ADC_v_v;
    }
    const std::vector<size_t> GetOutTicks() const {
      return _out_tick_v;
    }

    /// Get baseline histo
    const TH1D* GetBaseHisto() const { return _hInBase; }
    /// get baseline vector
    const std::vector<double> GetBase() const { return _in_base_v; }

    /// Get variance histo
    const TH1D* GetVarHisto() const { return _hInVar; }
    /// get variance vector
    const std::vector<double> GetVar() const { return _in_var_v; }

    /// get IDE histo
    const TH1D* GetIDEHisto() const { return _hIDEs; }
    /// get IDE vector
    const std::vector<double> GetIDE() const { return _in_IDE_v; }

    /// get event info
    int GetEvtNum() { return _evtNum; }
    int GetChan() { return _ch; }
    int GetPlane() { return _pl; }

    protected:

    /// Event Number
    int _evtNum;
    /// channel
    int _ch;
    /// plane
    int _pl;

    /// bool to suppress baseline or not
    bool _baseline;
    /// approximate baseline value
    double _base;

    /// Main Canvas
    TCanvas* _c1;
    /// Main Pad
    TPad* _p1;
    /// Original WF Histo
    TH1D* _hInWF;
    std::vector<double> _in_ADC_v;
    /// Original Baseline Histo (in blocks of 64)
    TH1D* _hInBase;
    std::vector<double> _in_base_v;
    /// Original Variance Histo (in blocks of 64)
    TH1D* _hInVar;
    std::vector<double> _in_var_v;
    /// Output WF Histo
    TH1D* _hOutWF;
    std::vector< std::vector<double> > _out_ADC_v_v;
    std::vector<size_t> _out_tick_v;
    /// IDE histogram
    TH1D* _hIDEs;
    std::vector<double> _in_IDE_v;

    /// Number of output waveforms from original input waveform
    int _NumOutWFs;

  };
}
#endif

/** @} */ // end of doxygen group 

/**
 * \file MakeFakeWF.h
 *
 * \ingroup Debug
 * 
 * \brief Class def header for a class MakeFakeWF
 *
 * @author david caratelli
 */

/** \addtogroup Debug

    @{*/

#ifndef LARLITE_MAKEFAKEWF_H
#define LARLITE_MAKEFAKEWF_H

#include "Analysis/ana_base.h"

#include <map>

namespace larlite {
  /**
     \class MakeFakeWF
     User custom analysis class made by SHELL_USER_NAME
   */
  class MakeFakeWF : public ana_base{
  
  public:

    /// Default constructor
    MakeFakeWF(){ _name="MakeFakeWF"; _fout=0;}

    /// Default destructor
    virtual ~MakeFakeWF(){}

    virtual bool initialize();

    virtual bool analyze(storage_manager* storage);

    virtual bool finalize();

    void addWaveform(const size_t& channel,const std::vector<short>& adc_v);

  protected:

    std::map<size_t, std::vector<short> > _wf_map;
    
    
  };
}
#endif

//**************************************************************************
// 
// For Analysis framework documentation, read Manual.pdf here:
//
// http://microboone-docdb.fnal.gov:8080/cgi-bin/ShowDocument?docid=3183
//
//**************************************************************************

/** @} */ // end of doxygen group 

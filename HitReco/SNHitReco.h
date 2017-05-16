/**
 * \file SNHitReco.h
 *
 * \ingroup HitReco
 * 
 * \brief Class def header for a class SNHitReco
 *
 * @author david caratelli
 */

/** \addtogroup HitReco

    @{*/

#ifndef LARLITE_SNHITRECO_H
#define LARLITE_SNHITRECO_H

#include "Analysis/ana_base.h"

namespace larlite {
  /**
     \class SNHitReco
     User custom analysis class made by SHELL_USER_NAME
   */
  class SNHitReco : public ana_base{
  
  public:

    /// Default constructor
    SNHitReco(){ _name="SNHitReco"; _fout=0;}

    /// Default destructor
    virtual ~SNHitReco(){}

    virtual bool initialize();

    virtual bool analyze(storage_manager* storage);

    virtual bool finalize();

    void SetWireProducer(std::string s) {_wire_producer = s; }
    void SetHitProducer (std::string s) {_hit_producer  = s; }
    void SetMinAmplitude(float n) { _min_ampl = n; }

  protected:

    std::string _wire_producer;
    std::string _hit_producer;

    float _min_ampl;
    
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

/**
 * \file HitMaker.h
 *
 * \ingroup Analysis
 * 
 * \brief Class def header for a class HitMaker
 *
 * @author SHELL_USER_NAME
 */

/** \addtogroup Analysis

    @{*/

#ifndef HITMAKER_H
#define HITMAKER_H

#include "Analysis/ana_base.h"
#include "LArUtil/Geometry.h"
#include "DataFormat/hit.h"
#include "DataFormat/rawdigit.h"

namespace larlite {
  /**
     \class HitMaker
     User custom analysis class made by SHELL_USER_NAME
   */
  class HitMaker : public ana_base{
  
  public:

    /// Default constructor
    HitMaker(){ _name="HitMaker"; _fout=0;};

    /// Default destructor
    virtual ~HitMaker(){};

    /** IMPLEMENT in HitMaker.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in HitMaker.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in HitMaker.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    protected:
    int    _evtNum;
    int    _baseline;
    int    _threshold;
    larlite::hit _Hit;
    
  };
}
#endif

/** @} */ // end of doxygen group 

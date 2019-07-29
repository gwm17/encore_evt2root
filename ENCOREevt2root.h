/* ENCOREevt2root.cpp 
 * class for converting nscldaq 11 .evt files to .root ROOT files for further analysis.
 * Utilizes nscldaq's data source and ring item methods (documentation found on http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/index.html)
 * This particular version is for the ENCORE setup at FSU.
 *
 * Gordon M. -- July 2019
 */

#ifndef ENCORE_EVT2ROOT_H
#define ENCORE_EVT2ROOT_H

#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>

#include "DataFormat.h"
#include "CDataSourceFactory.h"
#include "CDataSource.h"
#include "CRingItem.h"
#include "CRingItemFactory.h"
#include "CRingStateChangeItem.h"
#include "CPhysicsEventItem.h"
#include "CRingScalerItem.h"
#include "Exception.h"

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>

#include "ADCUnpacker.h"
#include "mADCUnpacker.h"

using namespace std;

class evt2root {
  public:
    evt2root();
    ~evt2root();
    void run(char *outname);
  
  private:
    int madc1_id, madc2_id, tdc_geo;
    vector<Int_t> madc1_values, madc2_values, tdc_values;
    vector<UInt_t> scalers;
    Float_t strip0;
    float         edepl[16];
    float         edepr[16];
    int           seg[16];
    Float_t         cath;
    Float_t         grid;
    Float_t         strip17;
    Float_t	   rf;
    Float_t	   mcp;
    Float_t	   frisch;
    Int_t scalerTag;
    vector<string> evt_list;
    vector<uint16_t> sample, exclude;
    void reset();
    void rebin(vector<Int_t>& module);
    bool initDataSource(string evtname);
    bool processSource();
    bool readFileList(string filename);
    void unpackPhysicsEvent(CPhysicsEventItem *phys_event);
    void unpackEnd(CRingStateChangeItem *end_event);
    void unpackBegin(CRingStateChangeItem *begin_event);
    void unpackScalers(CRingScalerItem *scaler_event);
    void getParameters();
    CDataSource *source;
    ADCUnpacker adc_unpacker;
    mADCUnpacker madc_unpacker;
    TRandom3 *random;
    TFile *output;
    TTree *DataTree;
    TTree *ScalerTree;

    Int_t RESET_VALUE = -10;
};


#endif

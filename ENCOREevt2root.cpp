/* ENCOREevt2root.cpp 
 * class for converting nscldaq 11 .evt files to .root ROOT files for further analysis.
 * Utilizes nscldaq's data source and ring item methods (documentation found on http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/index.html)
 * This particular version is for the ENCORE setup at FSU.
 *
 * Gordon M. -- July 2019
 */

#include "ENCOREevt2root.h"
#include <stdexcept>

using namespace std;

//constructor
evt2root::evt2root() {
  //sample and exclude not used in file conversion unless you really need speed
  sample.resize(0);
  exclude.resize(0);
  //resize module branch variables to appropriate channel numbers
  madc1_values.resize(32);
  madc2_values.resize(32);
  tdc_values.resize(32);

  //Source is set to NULL to avoid delete errors if there is unusual termination
  source = NULL;
  scalerTag = 0;
  madc1_id = 7;
  madc2_id = 9;
  tdc_geo = 16;
  random = new TRandom3();
}

evt2root::~evt2root() {
  delete random;
  delete source;
}

//read in a list of evt files
bool evt2root::readFileList(string filename) {
  ifstream input(filename);
  if(input.is_open()) {
    string evtname;
    while(input>>evtname) {
      evt_list.push_back(evtname);
    }
    return true;
  } else {
    cout<<"Error in ENCOREevt2root!!! File "<<filename<<" either cannnot be opened or doesn't exist!";
    return false;
  }
}

//initialize data source with nscldaq
bool evt2root::initDataSource(string evtname) {
  try {
    //source is dynamically allocated therefore must be deleted each time a new source is made
    if(source != NULL) delete source;
    source = CDataSourceFactory::makeSource(evtname, sample, exclude);
    return true;
  } catch(CException& error) {
    cout<<"Error in initDataSource!! Caught: "<<error.ReasonText()<<endl;
    cout<<"Skipping file "<<evtname<<endl;
    cout<<"-----------------------"<<endl;
    return false;
  }
}

//get all the data from the source
bool evt2root::processSource() {
  try {
    //physics event counter for progress update
    int physEvents = 0;
    while(true) {
      CRingItem *ring = source->getItem();
      //so according to nscl documentation source->getItem() will always give a pointer, even if there are no more ring items. When it reaches the end of a file
      //or a stop in the stream, it will give a NULL, so this becomes the break condition
      if(ring == NULL) {
        //nscl documentation says that when a NULL is given, errno should be checked to see if there is an error or if its the end of a file... but this isn't ideal as
        //errno is a global error parameter and could indicate an error completely unrelated to the source (or not even really indicate an error!)
        if(errno == 0) {
          cout<<"Exit successful without warnings"<<endl;
          cout<<"-----------------------"<<endl;
          break;
        } else {
          cout<<"Exit successful with warnings from  errno: "<<errno<<endl;
          cout<<"Continuing to run, check rootfile for buggy behavior after"<<endl; 
          cout<<"-----------------------"<<endl;
          break;
        }
      }
      switch(ring->type()) {
        case(PHYSICS_EVENT):
          {
            cout<<"\rNumber of Physics Events: "<<physEvents<<flush;
            CPhysicsEventItem *phys_event = reinterpret_cast<CPhysicsEventItem*>(ring);
            unpackPhysicsEvent(phys_event);
            physEvents++;
            break;
          }
        case(BEGIN_RUN):
          {
            CRingStateChangeItem *begin_event = reinterpret_cast<CRingStateChangeItem*>(ring);
            unpackBegin(begin_event);
            break;
          }
        case(END_RUN):
          {
            CRingStateChangeItem *end_event = reinterpret_cast<CRingStateChangeItem*>(ring);
            cout<<endl;
            unpackEnd(end_event);
            break;
          }
        case(PERIODIC_SCALERS):
          {
            CRingScalerItem *scaler_event = reinterpret_cast<CRingScalerItem*>(ring);
            unpackScalers(scaler_event);
            break;
          }
      }
      delete ring;
    }
    return true;
  } catch(CException& error) {
    cout<<"Error in processSource!! Caught: "<<error.ReasonText()<<endl;
    return false;
  }
}

//unpack physics event data; meat and potatoes of file conversion
void evt2root::unpackPhysicsEvent(CPhysicsEventItem* phys_event) {
  //first 16 bit word is the length of the event
  uint16_t *bodyPointer = (uint16_t*)phys_event->getBodyPointer();
  unsigned int size = (*bodyPointer++)/2;
  
  //get a 32 bit pointer to travel the event, and a pointer for the end
  uint32_t *iterPointer = (uint32_t*)bodyPointer;
  uint32_t *endPointer = iterPointer+size;
  vector<ParsedmADCEvent> madc_data;
  vector<ParsedADCEvent> adc_data;
  //reset branch values to avoid overfill on empty fields
  reset();
  
  //loop over length of event; looks to see if the current word matches the format of one of the modules, slower (slightly) than giving stack order but
  //this method requires NO knowledge of stack to unpack, so if you move modules around there is no impact on the unpacking process
  while(iterPointer<endPointer) {
    if(adc_unpacker.isHeader(*iterPointer)) {
      auto adc = adc_unpacker.parse(iterPointer, endPointer);
      adc_data.push_back(adc.second);
      iterPointer = adc.first;
    } else if(madc_unpacker.isHeader(*iterPointer)) {
      auto madc = madc_unpacker.parse(iterPointer, endPointer);
      madc_data.push_back(madc.second);
      iterPointer = madc.first;
    } else {
      iterPointer++;
    }
  }
  
  //sort into raw module branches
  for(auto& event:adc_data) {
    for(auto& chan_data:event.s_data) {
      if(event.s_geo == tdc_geo){ tdc_values[chan_data.first] = chan_data.second;
	if(chan_data.first == 0){rf = chan_data.second;}
	if(chan_data.first == 1){mcp = chan_data.second;}    
       }
     }
  }
  for(auto& event:madc_data) {
    for(auto& chan_data:event.s_data) {
      if(event.s_id == madc1_id){ madc1_values[chan_data.first] = chan_data.second;
      	if(chan_data.first == 0){strip0 = chan_data.second;}
	if(chan_data.first == 1){cath = chan_data.second;}
	if(chan_data.first == 2){grid = chan_data.second;}
	if(chan_data.first == 3){strip17 = chan_data.second;}
	}
      else if(event.s_id == madc2_id){ madc2_values[chan_data.first] = chan_data.second;
    	if(chan_data.first==0){edepl[15]=chan_data.second;}
	if(chan_data.first==1){edepl[0]=chan_data.second;}
	if(chan_data.first==2){edepl[14]=chan_data.second;}
	if(chan_data.first==3){edepl[1]=chan_data.second;}
	if(chan_data.first==4){edepl[13]=chan_data.second;}
	if(chan_data.first==5){edepl[2]=chan_data.second;}
	if(chan_data.first==6){edepl[12]=chan_data.second;}
	if(chan_data.first==7){edepl[3]=chan_data.second;}
	if(chan_data.first==8){edepl[11]=chan_data.second;}
	if(chan_data.first==9){edepl[4]=chan_data.second;}
	if(chan_data.first==10){edepl[10]=chan_data.second;}
	if(chan_data.first==11){edepl[5]=chan_data.second;}
	if(chan_data.first==12){edepl[9]=chan_data.second;}
	if(chan_data.first==13){edepl[6]=chan_data.second;}
	if(chan_data.first==14){edepl[8]=chan_data.second;}
	if(chan_data.first==15){edepl[7]=chan_data.second;}
//-------------------------------------------------------	
	if(chan_data.first==16){edepr[8]=chan_data.second;}
	if(chan_data.first==17){edepr[7]=chan_data.second;}
	if(chan_data.first==18){edepr[9]=chan_data.second;}
	if(chan_data.first==19){edepr[6]=chan_data.second;}
	if(chan_data.first==20){edepr[10]=chan_data.second;}
	if(chan_data.first==21){edepr[5]=chan_data.second;}
	if(chan_data.first==22){edepr[11]=chan_data.second;}
	if(chan_data.first==23){edepr[4]=chan_data.second;}
	if(chan_data.first==24){edepr[12]=chan_data.second;}
	if(chan_data.first==25){edepr[3]=chan_data.second;}
	if(chan_data.first==26){edepr[13]=chan_data.second;}
	if(chan_data.first==27){edepr[2]=chan_data.second;}
	if(chan_data.first==28){edepr[14]=chan_data.second;}
	if(chan_data.first==29){edepr[1]=chan_data.second;}
	if(chan_data.first==30){edepr[15]=chan_data.second;}
	if(chan_data.first==31){edepr[0]=chan_data.second;}
      }
      }
  }
   
  rebin(madc1_values); rebin(madc2_values); rebin(tdc_values);
  getParameters();
  DataTree->Fill();
  return;
}

//unpack scalers and fill out
void evt2root::unpackScalers(CRingScalerItem* scaler_event) {
  scalers = scaler_event->getScalers();
  ScalerTree->Fill();
  scalerTag++;
  return;
}

//unpack begin event for consistency check
void evt2root::unpackBegin(CRingStateChangeItem* begin_event) {
  cout<<"-----------------------"<<endl;
  cout<<"Converting Run: "<<begin_event->getRunNumber()<<endl;
  cout<<"Title: "<<begin_event->getTitle()<<endl;
  return;
}

//unpack end event for consistency check
void evt2root::unpackEnd(CRingStateChangeItem* end_event) {
  cout<<"End Run: "<<end_event->getRunNumber()<<endl;
  return;
}

//reset values to a dump to avoid overfill when a specific channel is unset
void evt2root::reset() {
  for(int i=0; i<32; i++) {
    tdc_values[i] = RESET_VALUE;
    madc1_values[i] = RESET_VALUE;
    madc2_values[i] = RESET_VALUE;
  }

  for(int i=0; i<16; i++){
	edepl[i] = RESET_VALUE;
	edepr[i] = RESET_VALUE;
  }

 strip0 = RESET_VALUE;
 strip17 = RESET_VALUE;
 grid = RESET_VALUE;
 cath = RESET_VALUE;
 rf = RESET_VALUE;
 mcp = RESET_VALUE;
  //RESET YOUR PARAMETERS HERE
  return;
}

//rebin raw modules to get rid of weird beating pattern in histograms
void evt2root::rebin(vector<Int_t> &module) {
  for(unsigned int i=0; i<module.size(); i++) {
    if(module[i] != RESET_VALUE) {
      module[i] = (Int_t)(module[i] + random->Rndm());
    }
  }
}

void evt2root::getParameters() {
//add your parameters here; also be sure to add them to the reset list!!

}

//loop over evt files
void evt2root::run(char *outname) {
  string file;
  bool errorFlag = true;
  cout<<"----ENCORE evt2root conversion----"<<endl;
  cout<<"Enter name of evt list file: ";
  cin>>file;
  cout<<"Beginning file conversion to "<<outname<<endl;

  output = new TFile(outname, "RECREATE");
  DataTree = new TTree("DataTree","DataTree");
  ScalerTree = new TTree("ScalerTree","ScalerTree");

  DataTree->Branch("madc1",&madc1_values);
  DataTree->Branch("madc2",&madc2_values);
  DataTree->Branch("scalerTag", &scalerTag, "scalerTag/I");
  DataTree->Branch("edepl",&edepl,"edepl[16]/F");
  DataTree->Branch("edepr",&edepr,"edepr[16]/F");
  DataTree->Branch("strip0",&strip0);
  DataTree->Branch("grid",&grid);
  DataTree->Branch("cath",&cath);
  DataTree->Branch("seg",&seg,"seg[16]/I");
  DataTree->Branch("strip17", &strip17);
  DataTree->Branch("rf", &rf);
  DataTree->Branch("mcp", &mcp);
  DataTree->Branch("frisch", &frisch);
  //add data branches here; not recommended to remove the raw module branches, as they are 
  //the easiest way to do debugging

  ScalerTree->Branch("scalers",&scalers);
  //add scaler branches here; again not recommended to remove the raw branch

  errorFlag = readFileList(file);
  if(errorFlag) {
    for(unsigned int i=0; i<evt_list.size(); i++) {
      errorFlag = initDataSource(evt_list[i]);
      if(errorFlag) {
        errorFlag = processSource();
        if(!errorFlag) break;
      }
    }
  }
}

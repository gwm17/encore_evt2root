/*mADCUnpacker.h
 *
 *Class to parse through data coming from Mesytec ADC. Loops through an event buffer and stores
 *data in a ParsecmADCEvent struct which is returned along with the location of the pointer that
 *traversed the data. This version is for 32 bit words 
 *Most of the program's structure borrowed from 
 *http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html
 *
 *
 *Gordon M. April 2019
 *
 *Updated July 2019 by Gordon M.
 *
 *See mesytec 32 channel ADC documentation for details on buffer structure
 */

#ifndef MADCUNPACKER_H 
#define MADCUNPACKER_H

#include <vector>
#include <utility>
#include <cstdint>
#include <stdexcept>

using namespace std;

struct ParsedmADCEvent {
  int s_id;
  int s_res; //Not actively used, but can be pulled if necessary
  int s_count;
  int s_eventNumber;   
  vector<pair<int, uint16_t>> s_data;
};


class mADCUnpacker {
  public:
    pair<uint32_t*, ParsedmADCEvent> parse(uint32_t* begin, uint32_t* end);
    bool isHeader(uint32_t word);

  private:
    bool isData(uint32_t word);
    bool isEOE(uint32_t word); 
   
    void unpackHeader(uint32_t* word, ParsedmADCEvent& event);
    void unpackDatum(uint32_t* word, ParsedmADCEvent& event); 
    uint32_t* unpackData(uint32_t* begin, uint32_t* end, ParsedmADCEvent& event); 
};


#endif
        

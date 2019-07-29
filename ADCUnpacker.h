/*ADCUnpacker.h
 *Class to parse through data coming from ADC 
 *Most of the program's structure borrowed from 
 *http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html
 *Ken H. & Gordon M.
 *Oct 2018
 *
 *This particular version is designed to work with ENCOREevt2root and uses 32-bit integer pointers 
 *to traverse, instead of the spectcl 32-bit translator pointers.
 *Gordon M. July 2019
 *
 *See CAEN 32 channel ADC documentation for detailed description of buffer structure
 */


#ifndef adcunpacker_h
#define adcunpacker_h

#include <vector>
#include <utility>
#include <cstdint>
#include <stdexcept>

using namespace std;

struct ParsedADCEvent {
  int s_geo;
  int s_crate;
  int s_count;
  int s_eventNumber;   
  vector<pair<int, uint16_t>> s_data;
};

class ADCUnpacker {
  public:
    pair<uint32_t*, ParsedADCEvent> parse(uint32_t* begin,uint32_t* end);
    bool isHeader(uint32_t word);

  private:
    bool isData(uint32_t word);
    bool isEOE(uint32_t word); 
   
    void unpackHeader(uint32_t* word, ParsedADCEvent& event);
    void unpackDatum(uint32_t* word, ParsedADCEvent& event); 
    uint32_t* unpackData(uint32_t* begin,uint32_t* end,ParsedADCEvent& event); 
};

#endif
        

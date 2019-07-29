/*ADCUnpacker.cpp
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

#include "ADCUnpacker.h"
#include <string>
#include <iostream>

using namespace std;

//This is where most chagnes need to be made for each module; most else is just name changes
//useful masks and shifts for ADC:
static const uint32_t TYPE_MASK (0x07000000);
static const uint32_t TYPE_HDR (0x02000000);
static const uint32_t TYPE_DATA (0x00000000);
static const uint32_t TYPE_TRAIL (0x04000000);

static const unsigned GEO_SHIFT (27);
static const uint32_t GEO_MASK (0xf8000000);

//header-specific:
static const unsigned HDR_COUNT_SHIFT (8);
static const uint32_t HDR_COUNT_MASK (0x00003f00);
static const unsigned HDR_CRATE_SHIFT (16);
static const uint32_t HDR_CRATE_MASK (0x00ff0000);

//data-specific:
static const unsigned DATA_CHANSHIFT (16);
static const uint32_t DATA_CHANMASK (0x001f0000);
static const unsigned DATA_CONVSHIFT(0);
static const uint32_t DATA_CONVMASK (0x00003fff);


pair<uint32_t*,ParsedADCEvent> ADCUnpacker::parse(uint32_t* begin,uint32_t* end) {

  ParsedADCEvent event;
  int bad_flag = 0;
  auto iter = begin;
  unpackHeader(iter, event);
  if (iter>end)  {
    bad_flag = 1;
  }
  iter++;
  int nWords = event.s_count;
  auto dataEnd = iter + nWords;
  if(dataEnd > end) {
    bad_flag = 1;
  } else {
    iter = unpackData(iter, dataEnd, event);
  }
  if (iter>end || bad_flag || !isEOE(*(iter))){
    cout<<"ADCUnpacker::parse() ";
    cout<<"Unable to unpack event"<<endl;
  }
  iter++;

  return make_pair(iter, event);

}

bool ADCUnpacker::isHeader(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void ADCUnpacker::unpackHeader(uint32_t* word, ParsedADCEvent& event) {

  //Error handling: if not valid header throw event to 0 at chan 0 at not real geo  
  try {
    if (!isHeader(*(word))) {
      string errmsg("ADCUnpacker::parseHeader() ");
      errmsg += "Found non-header word when expecting header. ";
      errmsg += "Word = ";
      unsigned int w = *(word);
      errmsg += to_string(w);
      throw errmsg;
    }
    event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
    event.s_geo = (*word&GEO_MASK)>>GEO_SHIFT;
    event.s_crate = (*word&HDR_CRATE_MASK) >> HDR_CRATE_SHIFT;
  } catch (string errmsg) {
    event.s_count = 0;
    event.s_geo = 99; //should NEVER match a valid geo
    event.s_crate = 0;
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    cout<<errmsg<<endl; 
  }
}

bool ADCUnpacker::isData(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA);
}

void ADCUnpacker::unpackDatum(uint32_t* word, ParsedADCEvent& event) {
  
  try {
    if (!isData(*(word))) {
      string errmsg("ADCUnpacker::unpackDatum() ");
      errmsg += "Found non-data word when expecting data.";
      throw errmsg;
    }
    uint16_t test_geo = (*word&GEO_MASK)>>GEO_SHIFT;
    if(test_geo != event.s_geo) {
      string errmsg("ADCUnpacker::unpackDatum() ");
      errmsg+="Found non-matching geoaddress when unpacking data.";
      throw errmsg;
    }
    uint16_t data = (*word&DATA_CONVMASK)>>DATA_CONVSHIFT;
    int channel = (*word&DATA_CHANMASK) >> DATA_CHANSHIFT;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
  } catch(string errmsg) {
    event.s_crate = 0;
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    cout<<errmsg<<endl;
  }
  
}

uint32_t* ADCUnpacker::unpackData(uint32_t* begin,uint32_t* end, ParsedADCEvent& event) {

  event.s_data.reserve(event.s_count); //memory allocation

  auto iter = begin;
  while (iter!=end) {
      unpackDatum(iter, event);
      iter = iter+1;
  }

  return iter;

}

bool ADCUnpacker::isEOE(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}



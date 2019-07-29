/*mADCUnpacker.cpp
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

#include "mADCUnpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

//This is the main place where changes need to be made from one module to another; all else mostly name changes
//useful masks and shifts for mADC:
//here we have to read as 0xf000 instead of 0xc000 due to some errors in evt files
static const uint32_t TYPE_MASK (0xf0000000);
static const uint32_t TYPE_HDR (0x40000000);
static const uint32_t TYPE_DATA (0x00000000);
static const uint32_t TYPE_TRAIL (0xc0000000);


//header-specific:
static const unsigned HDR_SUB_SHIFT(24);
static const uint32_t HDR_SUB_MASK(0x03f00000);
static const unsigned HDR_ID_SHIFT (16);
static const uint32_t HDR_ID_MASK (0x00ff0000);
static const unsigned HDR_COUNT_SHIFT (0);
static const uint32_t HDR_COUNT_MASK (0x00000fff);


//data-specific:
static const unsigned DATA_CHANSHIFT (16);
static const uint32_t DATA_CHANMASK (0x001f0000);
static const uint32_t DATA_CONVMASK (0x00000fff);

pair<uint32_t*, ParsedmADCEvent> mADCUnpacker::parse(uint32_t* begin, uint32_t* end) {

  ParsedmADCEvent event;

  auto iter = begin;
  int bad_flag = 0;
  unpackHeader(iter, event);
  if (iter > end){
    bad_flag =1;
  }
  iter++;
 

  int nWords = (event.s_count-1); //count includes the eoe 
  auto dataEnd = iter + nWords;
  if (dataEnd>end) {
    bad_flag = 1;
    cout<<"dataEnd > end error"<<endl;
  } else {
    iter = unpackData(iter, dataEnd, event);
  }

  if(bad_flag || iter>end || !isEOE(*iter)) {
    cout<<"mADCUpacker::parse() Unable to unpack event!"<<endl;
    cout<<"Word: "<<*iter<<endl;
  }

  iter++;

  return make_pair(iter, event);

}

bool mADCUnpacker::isHeader(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void mADCUnpacker::unpackHeader(uint32_t* word, ParsedmADCEvent& event) {
  try {
    if (!isHeader(*(word)) && (((*word)&HDR_SUB_MASK)>>HDR_SUB_SHIFT == 0)) {
      string errmsg("mADCUnpacker::parseHeader() ");
      errmsg += "Found non-header word when expecting header. ";
      errmsg += "Word = ";
      unsigned short w = *(word);
      errmsg += to_string(w);
      throw errmsg;
    }

    event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
    event.s_id = (*word&HDR_ID_MASK)>>HDR_ID_SHIFT;
  } catch (string errmsg) {
    event.s_count = 1;
    event.s_id = 99; //should NEVER match a valid id 
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
    cout<<errmsg<<endl; //only turn on if testing
  }
}

bool mADCUnpacker::isData(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA );
}

void mADCUnpacker::unpackDatum(uint32_t* word, ParsedmADCEvent& event) {
  //Error handling: if not valid data, throw 0 in chan 0 
  try {
    if (!isData(*(word))) {
      string errmsg("mADCUnpacker::unpackDatum() ");
      errmsg += "Found non-data word when expecting data.";
      throw errmsg;
    }

    uint16_t data = *word&DATA_CONVMASK;
    int channel = (*word&DATA_CHANMASK) >> DATA_CHANSHIFT;
    auto chanData = make_pair(channel, data);
    event.s_data.push_back(chanData);
  } catch (string errmsg) {
    uint16_t data = 0;
    int channel = 0;
    auto chanData = make_pair(channel, data);
    event.s_id = 99; //should NEVER match a valid id
    event.s_data.push_back(chanData);
    cout<<errmsg<<endl; //only turn on if testing
  }
  
}

 uint32_t* mADCUnpacker::unpackData( uint32_t* begin, uint32_t* end, ParsedmADCEvent& event) {

  event.s_data.reserve(event.s_count+1); //memory allocation
  auto iter = begin;
  while (iter<end) {
    unpackDatum(iter, event);
    iter = iter+1;
  }

  return iter;

}

bool mADCUnpacker::isEOE(uint32_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}


